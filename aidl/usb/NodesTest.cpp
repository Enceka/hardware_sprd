#include <android-base/file.h>
#include <gtest/gtest.h>
#include <sys/stat.h>
#include <unistd.h>

#include "Nodes.h"

namespace sprd::usb {

TEST(Nodes, PortNames) {
    EXPECT_TRUE(validPortName("port0"));
    EXPECT_TRUE(validPortName("port12"));
    for (const std::string name :
         {"", "port", "port-1", "port0/../port1", "port0-partner", "port0.0", "port０"}) {
        EXPECT_FALSE(validPortName(name));
    }
    EXPECT_FALSE(validPortName(std::string("port0\0other", 11)));
}

TEST(Nodes, Roles) {
    Roles roles;
    ASSERT_TRUE(parseRoles("host [device]\n", &roles));
    EXPECT_EQ(roles.active, "device");
    EXPECT_EQ(roles.supported, (std::vector<std::string>{"host", "device"}));
    ASSERT_TRUE(parseRoles("[dual] source sink\n", &roles));
    EXPECT_EQ(roles.active, "dual");
    ASSERT_TRUE(parseRoles("sink\n", &roles));
    EXPECT_EQ(roles.active, "sink");
    for (const std::string text :
         {"", "[host", "host]", "[]", "[host] [device]", "host device", "[device] device"}) {
        EXPECT_FALSE(parseRoles(text, &roles));
    }
}

TEST(Nodes, DataReadback) {
    TemporaryFile file;
    bool enabled;
    ASSERT_TRUE(android::base::WriteStringToFile("1\n", file.path));
    ASSERT_TRUE(readEnabled(file.path, &enabled));
    EXPECT_TRUE(enabled);
    ASSERT_TRUE(android::base::WriteStringToFile("0\n", file.path));
    ASSERT_TRUE(readEnabled(file.path, &enabled));
    EXPECT_FALSE(enabled);
    for (const std::string text : {"", "2", "true", "1 0", "01"}) {
        ASSERT_TRUE(android::base::WriteStringToFile(text, file.path));
        EXPECT_FALSE(readEnabled(file.path, &enabled));
    }
    EXPECT_FALSE(readEnabled(std::string(file.path) + "/missing", &enabled));
    Roles roles;
    ASSERT_TRUE(android::base::WriteStringToFile("source [sink]\n", file.path));
    ASSERT_TRUE(readRoles(file.path, &roles));
    EXPECT_EQ(roles.active, "sink");
    ASSERT_TRUE(android::base::WriteStringToFile("[broken", file.path));
    EXPECT_FALSE(readRoles(file.path, &roles));
    ASSERT_EQ(chmod(file.path, 0400), 0);
    EXPECT_FALSE(writable(file.path));
}

TEST(Nodes, Enumeration) {
    TemporaryDir directory;
    const std::string root(directory.path);
    std::vector<std::string> ports;
    ASSERT_TRUE(listPorts(root, &ports));
    EXPECT_TRUE(ports.empty());
    ASSERT_EQ(mkdir((root + "/port0").c_str(), 0700), 0);
    ASSERT_EQ(mkdir((root + "/port0-partner").c_str(), 0700), 0);
    ASSERT_EQ(mkdir((root + "/port1.0").c_str(), 0700), 0);
    ASSERT_EQ(symlink("port0", (root + "/port2").c_str()), 0);
    ASSERT_TRUE(listPorts(root, &ports));
    EXPECT_EQ(ports, (std::vector<std::string>{"port0", "port2"}));
    EXPECT_FALSE(listPorts(root + "/missing", &ports));
}

TEST(Nodes, Uevents) {
    constexpr char typec[] = "change@/devices/port0\0SUBSYSTEM=typec\0";
    constexpr char data[] = "change@/devices/usb_control\0SUBSYSTEM=usb_notify\0";
    constexpr char other[] = "change@/devices/test\0SUBSYSTEM=not_typec\0";
    constexpr char truncated[] = "SUBSYSTEM=typec";
    constexpr char tail[] = "SUBSYSTEM=typec\0TRUNCATED";
    EXPECT_TRUE(usbUevent(typec, sizeof(typec)));
    EXPECT_TRUE(usbUevent(data, sizeof(data)));
    EXPECT_FALSE(usbUevent(other, sizeof(other)));
    EXPECT_FALSE(usbUevent(truncated, sizeof(truncated) - 1));
    EXPECT_FALSE(usbUevent(tail, sizeof(tail) - 1));
}

TEST(Nodes, PortWithoutPortType) {
    TemporaryDir directory;
    const std::string root(directory.path);
    const std::string port = root + "/port0";
    ASSERT_EQ(mkdir(port.c_str(), 0700), 0);
    ASSERT_TRUE(android::base::WriteStringToFile("[host] device\n", port + "/data_role"));
    std::vector<std::string> ports;
    ASSERT_TRUE(listPorts(root, &ports));
    ASSERT_EQ(ports, (std::vector<std::string>{"port0"}));
    ModeSupport support;
    for (const std::string text : {"[source] sink\n", "source [sink]\n"}) {
        ASSERT_TRUE(android::base::WriteStringToFile(text, port + "/power_role"));
        ASSERT_TRUE(readModeSupport(port, &support));
        EXPECT_TRUE(support.source);
        EXPECT_TRUE(support.sink);
        EXPECT_FALSE(support.canChange);
    }
    ASSERT_TRUE(android::base::WriteStringToFile("[source]\n", port + "/power_role"));
    ASSERT_TRUE(readModeSupport(port, &support));
    EXPECT_TRUE(support.source);
    EXPECT_FALSE(support.sink);
    EXPECT_FALSE(support.canChange);
    ASSERT_TRUE(android::base::WriteStringToFile("[sink]\n", port + "/power_role"));
    ASSERT_TRUE(readModeSupport(port, &support));
    EXPECT_FALSE(support.source);
    EXPECT_TRUE(support.sink);
    EXPECT_FALSE(support.canChange);
    for (const std::string text : {"", "source sink", "[dual]", "[invalid]"}) {
        ASSERT_TRUE(android::base::WriteStringToFile(text, port + "/power_role"));
        EXPECT_FALSE(readModeSupport(port, &support));
    }
    ASSERT_EQ(unlink((port + "/power_role").c_str()), 0);
    EXPECT_FALSE(readModeSupport(port, &support));
}

TEST(Nodes, OptionalPortTypeErrors) {
    TemporaryDir directory;
    const std::string port(directory.path);
    ASSERT_TRUE(android::base::WriteStringToFile("[source] sink\n", port + "/power_role"));
    ASSERT_TRUE(android::base::WriteStringToFile("[dual] source sink\n", port + "/port_type"));
    ModeSupport support;
    ASSERT_TRUE(readModeSupport(port, &support));
    EXPECT_TRUE(support.source);
    EXPECT_TRUE(support.sink);
    EXPECT_TRUE(support.canChange);
    ASSERT_EQ(chmod((port + "/port_type").c_str(), 0444), 0);
    ASSERT_TRUE(readModeSupport(port, &support));
    EXPECT_FALSE(support.canChange);
    ASSERT_EQ(chmod((port + "/port_type").c_str(), 0600), 0);
    for (const std::string text : {"", "[invalid]", "[dual", "[source] [sink]"}) {
        ASSERT_TRUE(android::base::WriteStringToFile(text, port + "/port_type"));
        EXPECT_FALSE(readModeSupport(port, &support));
    }
    ASSERT_EQ(unlink((port + "/port_type").c_str()), 0);
    ASSERT_EQ(mkdir((port + "/port_type").c_str(), 0700), 0);
    EXPECT_FALSE(readModeSupport(port, &support));
    ASSERT_EQ(rmdir((port + "/port_type").c_str()), 0);
    ASSERT_EQ(symlink("port_type", (port + "/port_type").c_str()), 0);
    EXPECT_FALSE(readModeSupport(port, &support));
    ASSERT_EQ(unlink((port + "/port_type").c_str()), 0);
    ASSERT_EQ(symlink("missing", (port + "/port_type").c_str()), 0);
    EXPECT_FALSE(readModeSupport(port, &support));
}

}
