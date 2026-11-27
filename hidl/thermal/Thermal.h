// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <android/hardware/thermal/2.0/IThermal.h>

#include <mutex>
#include <condition_variable>
#include <map>
#include <thread>
#include <vector>

namespace android::hardware::thermal::V2_0::implementation {

class Thermal final : public IThermal {
  public:
    Thermal();
    ~Thermal() override;
    Return<void> getTemperatures(getTemperatures_cb callback) override;
    Return<void> getCpuUsages(getCpuUsages_cb callback) override;
    Return<void> getCoolingDevices(getCoolingDevices_cb callback) override;
    Return<void> getCurrentTemperatures(bool filterType, TemperatureType type,
                                        getCurrentTemperatures_cb callback) override;
    Return<void> getTemperatureThresholds(bool filterType, TemperatureType type,
                                          getTemperatureThresholds_cb callback) override;
    Return<void> registerThermalChangedCallback(
        const sp<IThermalChangedCallback>& callback, bool filterType, TemperatureType type,
        registerThermalChangedCallback_cb result) override;
    Return<void> unregisterThermalChangedCallback(
        const sp<IThermalChangedCallback>& callback,
        unregisterThermalChangedCallback_cb result) override;
    Return<void> getCurrentCoolingDevices(bool filterType, CoolingType type,
                                          getCurrentCoolingDevices_cb callback) override;

  private:
    struct Callback {
        sp<IThermalChangedCallback> interface;
        bool filterType;
        TemperatureType type;
    };

    std::mutex mMutex;
    std::condition_variable mCv;
    bool mStop = false;
    std::thread mMonitor;
    std::map<std::string, ThrottlingSeverity> mLastSeverity;
    std::vector<Callback> mCallbacks;

    void Monitor();
};

}  // namespace android::hardware::thermal::V2_0::implementation
