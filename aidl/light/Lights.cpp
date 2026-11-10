// SPDX-License-Identifier: Apache-2.0
#include "Lights.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/parseint.h>
#include <android-base/strings.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>

#include "Backlight.h"

namespace aidl::android::hardware::light {
namespace {
constexpr char kBacklight[] = "/sys/class/backlight/sprd_backlight";
constexpr char kButtons[] = "/sys/class/leds/keyboard-backlight";
}  // namespace

Lights::Lights() : mLights([] {
    // Always expose the panel, including if its module is still probing.
    // Probe/read its range on each write so an early boot race can recover.
    std::vector<Light> lights = {{{.id = 0, .ordinal = 0, .type = LightType::BACKLIGHT},
                                  kBacklight}};
    if (access((std::string(kButtons) + "/brightness").c_str(), F_OK) == 0) {
        lights.push_back({{.id = 1, .ordinal = 0, .type = LightType::BUTTONS}, kButtons});
    }
    return lights;
}()) {}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight>* lights) {
    lights->clear();
    for (const auto& light : mLights) lights->push_back(light.info);
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    const auto light = std::find_if(mLights.begin(), mLights.end(),
                                    [id](const Light& entry) { return entry.info.id == id; });
    if (light == mLights.end()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    std::lock_guard lock(mMutex);
    std::string value;
    int maximum = 0;
    if (!::android::base::ReadFileToString(light->directory + "/max_brightness", &value)) {
        const int error = errno;
        PLOG(ERROR) << "Cannot read " << light->directory << "/max_brightness";
        return ndk::ScopedAStatus::fromServiceSpecificError(error);
    }
    if (!::android::base::ParseInt(::android::base::Trim(value), &maximum) || maximum <= 0) {
        LOG(ERROR) << "Invalid max_brightness at " << light->directory;
        return ndk::ScopedAStatus::fromServiceSpecificError(EINVAL);
    }

    const int level = ::sprd::light::brightness(static_cast<uint32_t>(state.color), maximum);
    if (!::android::base::WriteStringToFile(std::to_string(level),
                                           light->directory + "/brightness")) {
        const int error = errno;
        PLOG(ERROR) << "Cannot set " << light->directory << "/brightness";
        return ndk::ScopedAStatus::fromServiceSpecificError(error);
    }
    return ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::light
