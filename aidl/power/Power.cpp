// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include "Power.h"

#include <android-base/file.h>
#include <android-base/logging.h>
#include <unistd.h>

#include <chrono>

namespace aidl::android::hardware::power::sprd {
ndk::ScopedAStatus Power::setMode(Mode type, bool enabled) {
    std::lock_guard lock(mMutex);
    if (type == Mode::DOUBLE_TAP_TO_WAKE) {
        if (mDoubleTapWakeNode.empty())
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
        if (!::android::base::WriteStringToFile(enabled ? "1\n" : "0\n", mDoubleTapWakeNode)) {
            PLOG(ERROR) << "Failed to set double-tap wake";
            return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_STATE);
        }
        return ndk::ScopedAStatus::ok();
    }
    const auto hint = toString(type);
    if (type == Mode::INTERACTIVE) {
        mInteractive = enabled;
        if (!enabled) {
            for (auto boost : ndk::enum_range<Boost>()) mHints->EndHint(toString(boost));
            mHints->EndHint("LAUNCH");
        }
        if (enabled)
            mHints->EndHint("DISPLAY_INACTIVE");
        else if (mHints->IsHintSupported("DISPLAY_INACTIVE"))
            mHints->DoHint("DISPLAY_INACTIVE");
    }
    if (type == Mode::LOW_POWER) {
        mLowPower = enabled;
        if (enabled) {
            for (auto boost : ndk::enum_range<Boost>()) mHints->EndHint(toString(boost));
            mHints->EndHint("LAUNCH");
        }
    }
    if (!mHints->IsHintSupported(hint)) return ndk::ScopedAStatus::ok();
    if (type == Mode::LAUNCH && enabled && (!mInteractive || mLowPower))
        return ndk::ScopedAStatus::ok();
    if (enabled)
        mHints->DoHint(hint);
    else
        mHints->EndHint(hint);
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Power::isModeSupported(Mode type, bool *supported) {
    if (type == Mode::DOUBLE_TAP_TO_WAKE) {
        *supported = !mDoubleTapWakeNode.empty() && access(mDoubleTapWakeNode.c_str(), F_OK) == 0;
        return ndk::ScopedAStatus::ok();
    }
    *supported = mHints->IsHintSupported(toString(type)) ||
                 (type == Mode::INTERACTIVE && mHints->IsHintSupported("INTERACTION"));
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Power::setBoost(Boost type, int32_t durationMs) {
    std::lock_guard lock(mMutex);
    const auto hint = toString(type);
    if (!mHints->IsHintSupported(hint)) return ndk::ScopedAStatus::ok();
    if (durationMs < 0)
        mHints->EndHint(hint);
    else if (mInteractive && !mLowPower) {
        if (durationMs == 0)
            mHints->DoHint(hint);
        else
            mHints->DoHint(hint, std::chrono::milliseconds(durationMs));
    }
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Power::isBoostSupported(Boost type, bool *supported) {
    *supported = mHints->IsHintSupported(toString(type));
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus Power::createHintSession(int32_t, int32_t, const std::vector<int32_t> &, int64_t,
                                            std::shared_ptr<IPowerHintSession> *session) {
    *session = nullptr;
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}
ndk::ScopedAStatus Power::getHintSessionPreferredRate(int64_t *rate) {
    *rate = -1;
    return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
}
binder_status_t Power::dump(int fd, const char **, uint32_t) {
    mHints->DumpToFd(fd);
    return STATUS_OK;
}
}  // namespace aidl::android::hardware::power::sprd
