/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.sprd.hardware.boot@1.2-service.stub"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include <vendor/sprd/hardware/boot/1.2/IBootControl.h>

namespace ns0 = ::vendor::sprd::hardware::boot::V1_2;

using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

struct Stub_boot_1_2_IBootControl : public ns0::IBootControl {};

int main() {
    configureRpcThreadpool(1, true /* callerWillJoin */);

    {
        sp<ns0::IBootControl> service = new Stub_boot_1_2_IBootControl();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IBootControl/default";
            return 1;
        }
    }

    joinRpcThreadpool();
    return 0;  // should never get here
}
