// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <aidl/android/hardware/power/BnPower.h>

#include <mutex>

#include "SceneManager.h"

namespace aidl::android::hardware::power::sprd {
class Power final : public BnPower {
   public:
    Power(std::shared_ptr<::sprd::power::SceneManager> hints, std::string doubleTapWakeNode,
          std::string interactiveNode = "")
        : mHints(std::move(hints)),
          mDoubleTapWakeNode(std::move(doubleTapWakeNode)),
          mInteractiveNode(std::move(interactiveNode)) {}
    ndk::ScopedAStatus setMode(Mode type, bool enabled) override;
    ndk::ScopedAStatus isModeSupported(Mode type, bool *supported) override;
    ndk::ScopedAStatus setBoost(Boost type, int32_t durationMs) override;
    ndk::ScopedAStatus isBoostSupported(Boost type, bool *supported) override;
    ndk::ScopedAStatus createHintSession(int32_t, int32_t, const std::vector<int32_t> &, int64_t,
                                         std::shared_ptr<IPowerHintSession> *session) override;
    ndk::ScopedAStatus getHintSessionPreferredRate(int64_t *rate) override;
    binder_status_t dump(int fd, const char **args, uint32_t numArgs) override;

   private:
    std::shared_ptr<::sprd::power::SceneManager> mHints;
    const std::string mDoubleTapWakeNode;
    const std::string mInteractiveNode;
    std::mutex mMutex;
    bool mInteractive = true;
    bool mLowPower = false;
};
}  // namespace aidl::android::hardware::power::sprd
