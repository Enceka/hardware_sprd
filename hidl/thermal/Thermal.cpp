// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include "Thermal.h"

#include <android-base/file.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <android/hardware/thermal/1.0/types.h>
#include <android/hardware/thermal/2.0/types.h>
#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <chrono>
#include <dirent.h>
#include <limits>
#include <string>
#include <vector>

namespace android::hardware::thermal::V2_0::implementation {
namespace {
using ::android::hardware::thermal::V1_0::CpuUsage;
using ::android::hardware::thermal::V1_0::ThermalStatus;
using ::android::hardware::thermal::V1_0::ThermalStatusCode;
using Temperature10 = ::android::hardware::thermal::V1_0::Temperature;
using CoolingDevice10 = ::android::hardware::thermal::V1_0::CoolingDevice;
using Temperature20 = ::android::hardware::thermal::V2_0::Temperature;
using CoolingDevice20 = ::android::hardware::thermal::V2_0::CoolingDevice;

struct Zone {
    std::string path;
    std::string name;
    TemperatureType type;
    float value;
    std::array<float, 7> hot;
    std::array<float, 7> cold;
    float vr;
    ThrottlingSeverity severity;
};

bool ReadInt(const std::string& path, int64_t* value) {
    std::string text;
    return ::android::base::ReadFileToString(path, &text) &&
           ::android::base::ParseInt(::android::base::Trim(text), value);
}

TemperatureType GetType(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    if (lower.find("battery") != std::string::npos || lower.find("batt") != std::string::npos)
        return TemperatureType::BATTERY;
    if (lower.find("gpu") != std::string::npos)
        return TemperatureType::GPU;
    if (lower.find("skin") != std::string::npos || lower.find("board") != std::string::npos)
        return TemperatureType::SKIN;
    if (lower.find("cpu") != std::string::npos || lower.find("core") != std::string::npos ||
        lower.find("cluster") != std::string::npos || lower.find("big") != std::string::npos ||
        lower.find("mid") != std::string::npos)
        return TemperatureType::CPU;
    if (lower.find("pa") != std::string::npos)
        return TemperatureType::POWER_AMPLIFIER;
    return TemperatureType::UNKNOWN;
}

std::vector<Zone> ReadZones() {
    std::vector<Zone> zones;
    DIR* directory = opendir("/sys/class/thermal");
    if (!directory) return zones;

    while (auto* entry = readdir(directory)) {
        const std::string entryName(entry->d_name);
        if (entryName.rfind("thermal_zone", 0) != 0 || entryName.size() == 12) continue;
        if (!std::all_of(entryName.begin() + 12, entryName.end(),
                         [](unsigned char c) { return std::isdigit(c); }))
            continue;

        const std::string path = "/sys/class/thermal/" + entryName;
        std::string name;
        int64_t rawValue;
        if (!::android::base::ReadFileToString(path + "/type", &name) ||
            !ReadInt(path + "/temp", &rawValue) || rawValue < -40000 || rawValue > 200000)
            continue;
        name = ::android::base::Trim(name);

        Zone zone{path,
                  name,
                  GetType(name),
                  static_cast<float>(rawValue) / 1000.0f,
                  {},
                  {},
                  std::numeric_limits<float>::quiet_NaN(),
                  ThrottlingSeverity::NONE};
        zone.hot.fill(std::numeric_limits<float>::quiet_NaN());
        zone.cold.fill(std::numeric_limits<float>::quiet_NaN());

        std::vector<std::pair<int64_t, std::string>> trips;
        for (int index = 0; index < 16; ++index) {
            const std::string tripPath = path + "/trip_point_" + std::to_string(index);
            int64_t tripValue;
            std::string tripType;
            if (!ReadInt(tripPath + "_temp", &tripValue) ||
                !::android::base::ReadFileToString(tripPath + "_type", &tripType))
                continue;
            trips.emplace_back(tripValue, ::android::base::Trim(tripType));
        }
        std::sort(trips.begin(), trips.end());
        size_t passive = 0;
        for (const auto& [tripValue, tripType] : trips) {
            if (tripValue < -40000 || tripValue > 200000) continue;
            const float celsius = static_cast<float>(tripValue) / 1000.0f;
            if (tripType == "critical") {
                zone.hot[static_cast<size_t>(ThrottlingSeverity::SHUTDOWN)] = celsius;
            } else if (tripType == "hot") {
                zone.hot[static_cast<size_t>(ThrottlingSeverity::EMERGENCY)] = celsius;
            } else if (tripType == "passive" || tripType == "active") {
                const auto severity = passive++ == 0 ? ThrottlingSeverity::MODERATE
                                                      : ThrottlingSeverity::SEVERE;
                zone.hot[static_cast<size_t>(severity)] = celsius;
            }
        }
        for (int level = static_cast<int>(ThrottlingSeverity::SHUTDOWN); level >= 0; --level) {
            if (std::isfinite(zone.hot[static_cast<size_t>(level)]) &&
                zone.value >= zone.hot[static_cast<size_t>(level)]) {
                zone.severity = static_cast<ThrottlingSeverity>(level);
                break;
            }
        }
        zones.push_back(std::move(zone));
    }
    closedir(directory);
    std::sort(zones.begin(), zones.end(), [](const Zone& a, const Zone& b) {
        return a.path < b.path;
    });
    return zones;
}

bool Matches(bool filterType, TemperatureType requested, TemperatureType actual) {
    return !filterType || requested == actual;
}

ThermalStatus Success() {
    return {ThermalStatusCode::SUCCESS, ""};
}

TemperatureThreshold Threshold(const Zone& zone) {
    TemperatureThreshold threshold{};
    threshold.type = zone.type;
    threshold.name = zone.name;
    for (size_t index = 0; index < zone.hot.size(); ++index) {
        threshold.hotThrottlingThresholds[index] = zone.hot[index];
        threshold.coldThrottlingThresholds[index] = zone.cold[index];
    }
    threshold.vrThrottlingThreshold = zone.vr;
    return threshold;
}

Temperature20 Temperature(const Zone& zone) {
    return {zone.type, zone.name, zone.value, zone.severity};
}

}  // namespace

Thermal::Thermal() {
    mMonitor = std::thread(&Thermal::Monitor, this);
}

Thermal::~Thermal() {
    {
        std::lock_guard lock(mMutex);
        mStop = true;
    }
    mCv.notify_all();
    if (mMonitor.joinable()) mMonitor.join();
}

void Thermal::Monitor() {
    std::unique_lock lock(mMutex);
    while (!mStop) {
        lock.unlock();
        const auto zones = ReadZones();
        lock.lock();
        std::vector<std::pair<sp<IThermalChangedCallback>, Temperature20>> notifications;
        for (const auto& zone : zones) {
            const auto previous = mLastSeverity.find(zone.name);
            if (previous != mLastSeverity.end() && previous->second == zone.severity) continue;
            mLastSeverity[zone.name] = zone.severity;
            const auto temperature = Temperature(zone);
            for (const auto& callback : mCallbacks)
                if (Matches(callback.filterType, callback.type, zone.type))
                    notifications.emplace_back(callback.interface, temperature);
        }
        lock.unlock();
        for (const auto& [callback, temperature] : notifications)
            callback->notifyThrottling(temperature);
        lock.lock();
        mCv.wait_for(lock, std::chrono::seconds(1), [this] { return mStop; });
    }
}

Return<void> Thermal::getTemperatures(getTemperatures_cb callback) {
    std::vector<Temperature10> temperatures;
    for (const auto& zone : ReadZones()) {
        temperatures.push_back({static_cast<::android::hardware::thermal::V1_0::TemperatureType>(
                                    zone.type),
                                zone.name,
                                zone.value,
                                zone.hot[static_cast<size_t>(ThrottlingSeverity::MODERATE)],
                                zone.hot[static_cast<size_t>(ThrottlingSeverity::SHUTDOWN)],
                                zone.vr});
    }
    callback(Success(), temperatures);
    return Void();
}

Return<void> Thermal::getCpuUsages(getCpuUsages_cb callback) {
    callback(Success(), std::vector<CpuUsage>{});
    return Void();
}

Return<void> Thermal::getCoolingDevices(getCoolingDevices_cb callback) {
    callback(Success(), std::vector<CoolingDevice10>{});
    return Void();
}

Return<void> Thermal::getCurrentTemperatures(bool filterType, TemperatureType type,
                                             getCurrentTemperatures_cb callback) {
    std::vector<Temperature20> temperatures;
    for (const auto& zone : ReadZones())
        if (Matches(filterType, type, zone.type)) temperatures.push_back(Temperature(zone));
    callback(Success(), temperatures);
    return Void();
}

Return<void> Thermal::getTemperatureThresholds(bool filterType, TemperatureType type,
                                               getTemperatureThresholds_cb callback) {
    std::vector<TemperatureThreshold> thresholds;
    for (const auto& zone : ReadZones())
        if (Matches(filterType, type, zone.type)) thresholds.push_back(Threshold(zone));
    callback(Success(), thresholds);
    return Void();
}

Return<void> Thermal::registerThermalChangedCallback(
    const sp<IThermalChangedCallback>& callback, bool filterType, TemperatureType type,
    registerThermalChangedCallback_cb result) {
    ThermalStatus status = Success();
    if (callback == nullptr) {
        status = {ThermalStatusCode::FAILURE, "Thermal callback is null"};
    } else {
        std::lock_guard lock(mMutex);
        for (const auto& registered : mCallbacks) {
            if (::android::hardware::interfacesEqual(registered.interface, callback)) {
                status = {ThermalStatusCode::FAILURE, "Thermal callback already registered"};
                break;
            }
        }
        if (status.code == ThermalStatusCode::SUCCESS)
            mCallbacks.push_back({callback, filterType, type});
    }
    if (status.code == ThermalStatusCode::SUCCESS) {
        const auto zones = ReadZones();
        for (const auto& zone : zones)
            if (Matches(filterType, type, zone.type)) callback->notifyThrottling(Temperature(zone));
    }
    result(status);
    return Void();
}

Return<void> Thermal::unregisterThermalChangedCallback(
    const sp<IThermalChangedCallback>& callback, unregisterThermalChangedCallback_cb result) {
    ThermalStatus status = {ThermalStatusCode::FAILURE, "Thermal callback not registered"};
    if (callback != nullptr) {
        std::lock_guard lock(mMutex);
        const auto oldSize = mCallbacks.size();
        mCallbacks.erase(std::remove_if(mCallbacks.begin(), mCallbacks.end(), [&](const auto& item) {
                             return ::android::hardware::interfacesEqual(item.interface, callback);
                         }),
                         mCallbacks.end());
        if (mCallbacks.size() != oldSize) status = Success();
    }
    result(status);
    return Void();
}

Return<void> Thermal::getCurrentCoolingDevices(bool, CoolingType,
                                               getCurrentCoolingDevices_cb callback) {
    callback(Success(), std::vector<CoolingDevice20>{});
    return Void();
}

}  // namespace android::hardware::thermal::V2_0::implementation
