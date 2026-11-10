// SPDX-License-Identifier: Apache-2.0
#pragma once

#include <aidl/android/hardware/memtrack/BnMemtrack.h>

namespace aidl::android::hardware::memtrack {
class Memtrack : public BnMemtrack {
  public:
    ndk::ScopedAStatus getMemory(int32_t pid, MemtrackType type,
                                 std::vector<MemtrackRecord>* records) override;
    ndk::ScopedAStatus getGpuDeviceInfo(std::vector<DeviceInfo>* devices) override;
};
}  // namespace aidl::android::hardware::memtrack
