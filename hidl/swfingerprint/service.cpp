/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.sw.swfingerprint@1.0-service.stub"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include <vendor/sw/swfingerprint/1.0/ISwfingerprint.h>

namespace ns0 = ::vendor::sw::swfingerprint::V1_0;

using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

struct Stub_1_0_ISwfingerprint : public ns0::ISwfingerprint {};

int main() {
    configureRpcThreadpool(1, true /* callerWillJoin */);

    {
        sp<ns0::ISwfingerprint> service = new Stub_1_0_ISwfingerprint();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register ISwfingerprint/default";
            return 1;
        }
    }

    joinRpcThreadpool();
    return 0;  // should never get here
}
