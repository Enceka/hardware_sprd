// SPDX-License-Identifier: Apache-2.0
#include "Memtrack.h"

#include <android-base/file.h>
#include <glob.h>
#include <unistd.h>

#include <cerrno>
#include <fstream>
#include <set>

#include "MaliMemory.h"

namespace aidl::android::hardware::memtrack {

ndk::ScopedAStatus Memtrack::getMemory(int32_t pid, MemtrackType type,
                                      std::vector<MemtrackRecord>* records) {
    records->clear();
    if (pid < 0) return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT);
    switch (type) {
        case MemtrackType::OTHER:
        case MemtrackType::GRAPHICS:
        case MemtrackType::MULTIMEDIA:
        case MemtrackType::CAMERA:
            // This kbase interface does not provide DMA-BUF PSS. Do not
            // report total DMA-BUF allocations as per-process usage.
            return pid == 0 ? ndk::ScopedAStatus::ok()
                           : ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
        case MemtrackType::GL:
            break;
        default:
            return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    std::ifstream input("/sys/kernel/debug/mali0/gpu_memory");
    if (!input.is_open()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    const auto bytes = ::sprd::memtrack::readMaliMemory(input, pid, sysconf(_SC_PAGESIZE));
    if (!bytes) return ndk::ScopedAStatus::fromServiceSpecificError(EIO);
    // kbase CPU mappings use VM_PFNMAP, so smaps does not account these pages.
    records->push_back({.flags = MemtrackRecord::FLAG_SMAPS_UNACCOUNTED |
                                MemtrackRecord::FLAG_PRIVATE,
                        .sizeInBytes = *bytes});
    return ndk::ScopedAStatus::ok();
}

ndk::ScopedAStatus Memtrack::getGpuDeviceInfo(std::vector<DeviceInfo>* devices) {
    devices->clear();
    std::set<std::string> names;
    // Report the kernel device name, not gpuinfo's marketing name. First
    // prefer miscdevice's parent; the platform driver is a fallback.
    for (const char* pattern : {"/sys/class/misc/mali*/device",
                               "/sys/bus/platform/drivers/mali/*.gpu"}) {
        glob_t paths{};
        if (glob(pattern, 0, nullptr, &paths) == 0) {
            for (size_t i = 0; i < paths.gl_pathc; ++i) {
                std::string path;
                if (::android::base::Realpath(paths.gl_pathv[i], &path)) {
                    names.insert(::android::base::Basename(path));
                }
            }
        }
        globfree(&paths);
        if (!names.empty()) break;
    }
    int32_t id = 0;
    for (const auto& name : names) devices->push_back({.id = id++, .name = name});
    return ndk::ScopedAStatus::ok();
}

}  // namespace aidl::android::hardware::memtrack
