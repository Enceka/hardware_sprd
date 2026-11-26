// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android/binder_stability.h>

#include <cstdlib>

#include "Power.h"
#include "VendorPower.h"

int main(int argc, char** argv) {
    ::android::base::InitLogging(argv, ::android::base::LogdLogger(::android::base::SYSTEM));
    const char* config = argc > 1 ? argv[1] : "/vendor/etc";
    auto hints = std::make_shared<::sprd::power::SceneManager>(
        [](const std::string& path, std::string* value) {
            return ::android::base::ReadFileToString(path, value);
        },
        [](const std::string& path, const std::string& value) {
            return ::android::base::WriteStringToFile(value, path);
        });
    CHECK(hints->Load(config)) << "Invalid Power HAL configuration: " << config;
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    auto power = ndk::SharedRefBase::make<aidl::android::hardware::power::sprd::Power>(hints);
    const std::string instance = std::string(power->descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(power->asBinder().get(), instance.c_str()), STATUS_OK);
    auto vendorPower = ndk::SharedRefBase::make<::sprd::power::VendorPower>(hints);
    const std::string vendorInstance = std::string(vendorPower->descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(vendorPower->asBinder().get(), vendorInstance.c_str()),
             STATUS_OK);
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;
}
