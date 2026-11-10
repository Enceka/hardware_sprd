// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <aidl/android/hardware/light/BnLights.h>
#include <mutex>
#include <string>
#include <vector>

namespace aidl::android::hardware::light {

class Lights : public BnLights {
  public:
    Lights();
    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* lights) override;

  private:
    struct Light {
        HwLight info;
        std::string directory;
    };
    const std::vector<Light> mLights;
    std::mutex mMutex;
};

}  // namespace aidl::android::hardware::light
