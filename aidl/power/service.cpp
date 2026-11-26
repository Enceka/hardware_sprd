// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <cstdlib>

#include "Power.h"

int main(int argc, char **argv) {
    ::android::base::InitLogging(argv, ::android::base::LogdLogger(::android::base::SYSTEM));
    const char *config = argc > 1 ? argv[1] : "/vendor/etc/powerhint.json";
    auto hints = ::android::perfmgr::HintManager::GetFromJSON(config);
    CHECK(hints != nullptr) << "Invalid Power HAL configuration: " << config;
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    auto power =
        ndk::SharedRefBase::make<aidl::android::hardware::power::sprd::Power>(std::move(hints));
    const std::string instance = std::string(power->descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(power->asBinder().get(), instance.c_str()), STATUS_OK);
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;
}
