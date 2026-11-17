#include "Usb.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/strings.h>
#include <cutils/uevent.h>
#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>

#include "Nodes.h"

namespace aidl::android::hardware::usb {

using ::android::base::ReadFileToString;
using ::android::base::Trim;
using ::android::base::WriteStringToFile;

namespace {
constexpr char kTypec[] = "/sys/class/typec";
constexpr char kData[] = "/sys/class/usb_notify/usb_control/usb_data_enabled";
constexpr char kMode[] = "/port_type";
constexpr char kPower[] = "/power_role";
constexpr char kDataRole[] = "/data_role";

std::string node(const std::string& port, const char* suffix) {
    return std::string(kTypec) + "/" + port + suffix;
}

PortPowerRole power(const std::string& value) {
    if (value == "source") return PortPowerRole::SOURCE;
    if (value == "sink") return PortPowerRole::SINK;
    return PortPowerRole::NONE;
}

PortDataRole dataRole(const std::string& value) {
    if (value == "host") return PortDataRole::HOST;
    if (value == "device") return PortDataRole::DEVICE;
    return PortDataRole::NONE;
}

Status parseRole(const PortRole& role, const char** path, const char** value) {
    switch (role.getTag()) {
        case PortRole::mode:
            *path = kMode;
            if (role.get<PortRole::mode>() == PortMode::DRP)
                *value = "dual";
            else if (role.get<PortRole::mode>() == PortMode::DFP)
                *value = "source";
            else if (role.get<PortRole::mode>() == PortMode::UFP)
                *value = "sink";
            else
                return Status::UNRECOGNIZED_ROLE;
            return Status::SUCCESS;
        case PortRole::powerRole:
            *path = kPower;
            if (role.get<PortRole::powerRole>() == PortPowerRole::SOURCE)
                *value = "source";
            else if (role.get<PortRole::powerRole>() == PortPowerRole::SINK)
                *value = "sink";
            else
                return Status::UNRECOGNIZED_ROLE;
            return Status::SUCCESS;
        case PortRole::dataRole:
            *path = kDataRole;
            if (role.get<PortRole::dataRole>() == PortDataRole::HOST)
                *value = "host";
            else if (role.get<PortRole::dataRole>() == PortDataRole::DEVICE)
                *value = "device";
            else
                return Status::UNRECOGNIZED_ROLE;
            return Status::SUCCESS;
    }
    return Status::UNRECOGNIZED_ROLE;
}
}

Usb::Usb() {
    mStop.reset(eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK));
    mUevent.reset(uevent_open_socket(64 * 1024, true));
    CHECK(mStop.ok() && mUevent.ok()) << "USB event monitoring unavailable";
    mMonitor = std::thread(&Usb::monitor, this);
}

Usb::~Usb() {
    const uint64_t stop = 1;
    if (mStop.ok()) write(mStop, &stop, sizeof(stop));
    if (mMonitor.joinable()) mMonitor.join();
}

std::shared_ptr<IUsbCallback> Usb::callback() {
    std::lock_guard lock(mCallbackMutex);
    return mCallback;
}

Status Usb::validatePort(const std::string& portName) {
    if (!sprd::usb::validPortName(portName)) return Status::INVALID_ARGUMENT;
    std::vector<std::string> ports;
    if (!sprd::usb::listPorts(kTypec, &ports)) return Status::ERROR;
    return std::find(ports.begin(), ports.end(), portName) == ports.end() ? Status::INVALID_ARGUMENT
                                                                          : Status::SUCCESS;
}

Status Usb::readStatus(std::vector<PortStatus>* ports) {
    std::vector<std::string> names;
    if (!sprd::usb::listPorts(kTypec, &names)) return Status::ERROR;
    bool enabled = false;
    if (!names.empty() && !sprd::usb::readEnabled(kData, &enabled)) return Status::ERROR;
    for (const auto& name : names) {
        sprd::usb::Roles powerRoles, dataRoles;
        sprd::usb::ModeSupport modes;
        if (!sprd::usb::readRoles(node(name, kPower), &powerRoles) ||
            !sprd::usb::readRoles(node(name, kDataRole), &dataRoles))
            return Status::ERROR;
        if (!sprd::usb::readModeSupport(node(name, ""), &modes)) return Status::ERROR;
        for (const auto& value : powerRoles.supported) {
            if (power(value) == PortPowerRole::NONE) return Status::ERROR;
        }
        for (const auto& value : dataRoles.supported) {
            if (dataRole(value) == PortDataRole::NONE) return Status::ERROR;
        }
        PortStatus status;
        status.portName = name;
        if (access(node(name, "-partner").c_str(), F_OK) == 0) {
            status.currentPowerRole = power(powerRoles.active);
            status.currentDataRole = dataRole(dataRoles.active);
            status.currentMode =
                status.currentDataRole == PortDataRole::HOST ? PortMode::DFP : PortMode::UFP;
            std::string accessory;
            if (!ReadFileToString(node(name, "-partner/accessory_mode"), &accessory))
                return Status::ERROR;
            if (Trim(accessory) == "analog_audio")
                status.currentMode = PortMode::AUDIO_ACCESSORY;
            else if (Trim(accessory) == "debug")
                status.currentMode = PortMode::DEBUG_ACCESSORY;
            else if (Trim(accessory) != "none")
                return Status::ERROR;
        }
        status.supportedModes.push_back(modes.source && modes.sink ? PortMode::DRP
                                        : modes.source            ? PortMode::DFP
                                                                  : PortMode::UFP);
        status.canChangeMode = modes.canChange;
        std::string pd;
        const bool supportsPd =
            ReadFileToString(node(name, "-partner/supports_usb_power_delivery"), &pd) &&
            Trim(pd) == "yes";
        status.canChangePowerRole = supportsPd && powerRoles.supported.size() > 1 &&
                                    sprd::usb::writable(node(name, kPower));
        status.canChangeDataRole = supportsPd && dataRoles.supported.size() > 1 &&
                                   sprd::usb::writable(node(name, kDataRole));
        status.supportedContaminantProtectionModes.push_back(ContaminantProtectionMode::NONE);
        status.contaminantProtectionStatus = ContaminantProtectionStatus::NONE;
        status.contaminantDetectionStatus = ContaminantDetectionStatus::NOT_SUPPORTED;
        status.usbDataStatus.push_back(names.size() != 1 ? UsbDataStatus::UNKNOWN
                                       : enabled         ? UsbDataStatus::ENABLED
                                                         : UsbDataStatus::DISABLED_FORCE);
        status.powerTransferLimited = false;
        status.powerBrickStatus = PowerBrickStatus::UNKNOWN;
        ports->push_back(std::move(status));
    }
    return Status::SUCCESS;
}

void Usb::publish(const std::shared_ptr<IUsbCallback>& client) {
    std::vector<PortStatus> ports;
    const Status result = readStatus(&ports);
    if (result != Status::SUCCESS) ports.clear();
    if (client) client->notifyPortStatusChange(ports, result);
}

ndk::ScopedAStatus Usb::setCallback(const std::shared_ptr<IUsbCallback>& callback) {
    {
        std::lock_guard lock(mCallbackMutex);
        mCallback = callback;
    }
    publish(callback);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Usb::queryPortStatus(int64_t transactionId) {
    auto client = callback();
    std::vector<PortStatus> ports;
    const Status result = readStatus(&ports);
    if (client) {
        if (result != Status::SUCCESS) ports.clear();
        client->notifyPortStatusChange(ports, result);
        client->notifyQueryPortStatus("", result, transactionId);
    }
    return ndk::ScopedAStatus::ok();
}

Status Usb::changeRole(const std::string& portName, const PortRole& role) {
    const char* path;
    const char* value;
    const Status parsed = parseRole(role, &path, &value);
    if (parsed != Status::SUCCESS) return parsed;
    std::lock_guard lock(mNodeMutex);
    const std::string file = node(portName, path);
    if (!sprd::usb::writable(file)) return Status::NOT_SUPPORTED;
    if (!WriteStringToFile(value, file)) return Status::ERROR;
    sprd::usb::Roles roles;
    for (int attempt = 0; attempt < 160; ++attempt) {
        bool matched = sprd::usb::readRoles(file, &roles) && roles.active == value;
        if (matched && role.getTag() == PortRole::mode &&
            role.get<PortRole::mode>() != PortMode::DRP) {
            matched =
                access(node(portName, "-partner").c_str(), F_OK) == 0 &&
                sprd::usb::readRoles(node(portName, kDataRole), &roles) &&
                roles.active == (role.get<PortRole::mode>() == PortMode::DFP ? "host" : "device");
        } else if (matched && role.getTag() != PortRole::mode) {
            matched = access(node(portName, "-partner").c_str(), F_OK) == 0;
        }
        if (matched) return Status::SUCCESS;
        usleep(50000);
    }
    return Status::ERROR;
}

ndk::ScopedAStatus Usb::switchRole(const std::string& portName, const PortRole& role,
                                   int64_t transactionId) {
    Status result = validatePort(portName);
    if (result == Status::SUCCESS) result = changeRole(portName, role);
    auto client = callback();
    if (client) client->notifyRoleSwitchStatus(portName, role, result, transactionId);
    if (result == Status::SUCCESS) publish(client);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Usb::enableUsbData(const std::string& portName, bool enable,
                                      int64_t transactionId) {
    Status result = validatePort(portName);
    if (result == Status::SUCCESS) {
        std::lock_guard lock(mNodeMutex);
        std::vector<std::string> ports;
        if (!sprd::usb::listPorts(kTypec, &ports))
            result = Status::ERROR;
        else if (ports.size() != 1)
            result = Status::NOT_SUPPORTED;
        else if (!sprd::usb::writable(kData) || !WriteStringToFile(enable ? "1" : "0", kData))
            result = Status::ERROR;
        bool actual;
        if (result == Status::SUCCESS &&
            (!sprd::usb::readEnabled(kData, &actual) || actual != enable))
            result = Status::ERROR;
    }
    auto client = callback();
    if (client) client->notifyEnableUsbDataStatus(portName, enable, result, transactionId);
    if (result == Status::SUCCESS) publish(client);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Usb::enableUsbDataWhileDocked(const std::string& portName,
                                                 int64_t transactionId) {
    auto client = callback();
    Status result = validatePort(portName);
    if (result == Status::SUCCESS) result = Status::NOT_SUPPORTED;
    if (client) client->notifyEnableUsbDataWhileDockedStatus(portName, result, transactionId);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Usb::enableContaminantPresenceDetection(const std::string& portName, bool enable,
                                                           int64_t transactionId) {
    auto client = callback();
    Status result = validatePort(portName);
    if (result == Status::SUCCESS) result = Status::NOT_SUPPORTED;
    if (client) client->notifyContaminantEnabledStatus(portName, enable, result, transactionId);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Usb::limitPowerTransfer(const std::string& portName, bool limit,
                                           int64_t transactionId) {
    auto client = callback();
    Status result = validatePort(portName);
    if (result == Status::SUCCESS) result = Status::NOT_SUPPORTED;
    if (client) client->notifyLimitPowerTransferStatus(portName, limit, result, transactionId);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Usb::resetUsbPort(const std::string& portName, int64_t transactionId) {
    auto client = callback();
    Status result = validatePort(portName);
    if (result == Status::SUCCESS) result = Status::NOT_SUPPORTED;
    if (client) client->notifyResetUsbPortStatus(portName, result, transactionId);
    return ndk::ScopedAStatus::ok();
}

void Usb::monitor() {
    char message[4096];
    pollfd descriptors[] = {{mStop.get(), POLLIN, 0}, {mUevent.get(), POLLIN, 0}};
    while (true) {
        const int result = poll(descriptors, 2, 1000);
        if (result < 0 && errno == EINTR) continue;
        if (result < 0 || descriptors[0].revents ||
            (descriptors[1].revents & (POLLERR | POLLHUP | POLLNVAL)))
            return;
        if (!(descriptors[1].revents & POLLIN)) continue;
        const ssize_t length = uevent_kernel_multicast_recv(mUevent, message, sizeof(message));
        if (length > 0 && length < static_cast<ssize_t>(sizeof(message)) &&
            sprd::usb::usbUevent(message, static_cast<size_t>(length)))
            publish(callback());
    }
}

}
