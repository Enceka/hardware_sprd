// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <aidl/android/hardware/power/BnPower.h>
#include <perfmgr/HintManager.h>

#include <mutex>

namespace aidl::android::hardware::power::sprd {
class Power final : public BnPower {
   public:
    explicit Power(std::unique_ptr<::android::perfmgr::HintManager> hints)
        : mHints(std::move(hints)) {}
    ndk::ScopedAStatus setMode(Mode type, bool enabled) override;
    ndk::ScopedAStatus isModeSupported(Mode type, bool *supported) override;
    ndk::ScopedAStatus setBoost(Boost type, int32_t durationMs) override;
    ndk::ScopedAStatus isBoostSupported(Boost type, bool *supported) override;
    ndk::ScopedAStatus createHintSession(int32_t, int32_t, const std::vector<int32_t> &, int64_t,
                                         std::shared_ptr<IPowerHintSession> *session) override;
    ndk::ScopedAStatus getHintSessionPreferredRate(int64_t *rate) override;
    binder_status_t dump(int fd, const char **args, uint32_t numArgs) override;

   private:
    std::unique_ptr<::android::perfmgr::HintManager> mHints;
    std::mutex mMutex;
    bool mInteractive = true;
    bool mLowPower = false;
};
}  // namespace aidl::android::hardware::power::sprd
