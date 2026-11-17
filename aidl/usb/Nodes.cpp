#include "Nodes.h"

#include <android-base/file.h>
#include <android-base/strings.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <memory>
#include <sstream>

namespace sprd::usb {

bool validPortName(const std::string& name) {
    return name.size() > 4 && name.size() <= 32 && name.compare(0, 4, "port") == 0 &&
           std::all_of(name.begin() + 4, name.end(),
                       [](char value) { return value >= '0' && value <= '9'; });
}

bool parseRoles(const std::string& text, Roles* roles) {
    Roles parsed;
    std::istringstream stream(text);
    std::string token;
    while (stream >> token) {
        if (token.front() == '[' && token.back() == ']') {
            if (!parsed.active.empty()) return false;
            token = token.substr(1, token.size() - 2);
            parsed.active = token;
        }
        if (token.empty() ||
            token.find_first_not_of("abcdefghijklmnopqrstuvwxyz_") != std::string::npos ||
            std::find(parsed.supported.begin(), parsed.supported.end(), token) !=
                parsed.supported.end()) {
            return false;
        }
        parsed.supported.push_back(token);
    }
    if (parsed.supported.empty()) return false;
    if (parsed.active.empty()) {
        if (parsed.supported.size() != 1) return false;
        parsed.active = parsed.supported.front();
    }
    *roles = std::move(parsed);
    return true;
}

bool readRoles(const std::string& path, Roles* roles) {
    std::string text;
    return android::base::ReadFileToString(path, &text) && parseRoles(text, roles);
}

bool readModeSupport(const std::string& portPath, ModeSupport* support) {
    const std::string path = portPath + "/port_type";
    std::string text;
    Roles roles;
    ModeSupport parsed;
    const bool hasPortType = android::base::ReadFileToString(path, &text);
    if (hasPortType) {
        if (!parseRoles(text, &roles)) return false;
        parsed.canChange = writable(path);
    } else {
        if (errno != ENOENT) return false;
        struct stat info {};
        if (lstat(path.c_str(), &info) == 0 || errno != ENOENT ||
            !readRoles(portPath + "/power_role", &roles)) return false;
    }
    for (const auto& value : roles.supported) {
        if (value == "source") parsed.source = true;
        else if (value == "sink") parsed.sink = true;
        else if (hasPortType && value == "dual") parsed.source = parsed.sink = true;
        else return false;
    }
    *support = parsed;
    return true;
}

bool readEnabled(const std::string& path, bool* enabled) {
    std::string text;
    if (!android::base::ReadFileToString(path, &text)) return false;
    text = android::base::Trim(text);
    if (text != "0" && text != "1") return false;
    *enabled = text == "1";
    return true;
}

bool listPorts(const std::string& root, std::vector<std::string>* ports) {
    ports->clear();
    std::unique_ptr<DIR, decltype(&closedir)> directory(opendir(root.c_str()), closedir);
    if (!directory) return false;
    errno = 0;
    while (auto* entry = readdir(directory.get())) {
        const std::string name(entry->d_name);
        if (!validPortName(name)) continue;
        struct stat info {};
        if (stat((root + "/" + name).c_str(), &info) != 0 || !S_ISDIR(info.st_mode)) {
            return false;
        }
        ports->push_back(name);
        errno = 0;
    }
    if (errno != 0) return false;
    std::sort(ports->begin(), ports->end());
    return true;
}

bool writable(const std::string& path) {
    struct stat info {};
    return stat(path.c_str(), &info) == 0 && (info.st_mode & 0222) != 0 &&
           access(path.c_str(), W_OK) == 0;
}

bool usbUevent(const char* message, size_t length) {
    size_t offset = 0;
    bool relevant = false;
    while (offset < length) {
        const size_t size = strnlen(message + offset, length - offset);
        if (size == length - offset) return false;
        const std::string field(message + offset, size);
        if (field == "SUBSYSTEM=typec" || field == "SUBSYSTEM=usb_notify" ||
            field == "SUBSYSTEM=usb" || field == "SUBSYSTEM=power_supply") {
            relevant = true;
        }
        offset += size + 1;
    }
    return relevant;
}

}
