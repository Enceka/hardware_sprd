/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.unisoc.hardware-aidl-service.stub"

#include <android-base/logging.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android/binder_status.h>
#include <cstdlib>
#include <string>

#include <aidl/vendor/unisoc/hardware/ai_engine/BnAIEngineControl.h>
#include <aidl/vendor/unisoc/hardware/power/BnVendorPower.h>

using ::aidl::vendor::unisoc::hardware::ai_engine::BnAIEngineControl;
using ::aidl::vendor::unisoc::hardware::power::BnVendorPower;

class Stub_vendor_unisoc_hardware_ai_engine_IAIEngineControl : public BnAIEngineControl {};
class Stub_vendor_unisoc_hardware_power_IVendorPower : public BnVendorPower {};

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_ai_engine_IAIEngineControl>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_ai_engine_IAIEngineControl::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_power_IVendorPower>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_power_IVendorPower::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
