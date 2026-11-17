#pragma once

#include <string>
#include <vector>

namespace sprd::usb {

struct Roles {
    std::string active;
    std::vector<std::string> supported;
};

struct ModeSupport {
    bool source = false;
    bool sink = false;
    bool canChange = false;
};

bool validPortName(const std::string& name);
bool parseRoles(const std::string& text, Roles* roles);
bool readRoles(const std::string& path, Roles* roles);
bool readModeSupport(const std::string& portPath, ModeSupport* support);
bool readEnabled(const std::string& path, bool* enabled);
bool listPorts(const std::string& root, std::vector<std::string>* ports);
bool writable(const std::string& path);
bool usbUevent(const char* message, size_t length);

}
