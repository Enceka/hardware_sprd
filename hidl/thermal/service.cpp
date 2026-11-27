// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include "Thermal.h"

int main() {
    using ::android::hardware::configureRpcThreadpool;
    using ::android::hardware::joinRpcThreadpool;
    using ::android::hardware::thermal::V2_0::IThermal;
    using ::android::hardware::thermal::V2_0::implementation::Thermal;

    configureRpcThreadpool(1, true);
    ::android::sp<IThermal> service = new Thermal();
    if (service->registerAsService() != ::android::OK) {
        LOG(ERROR) << "Failed to register SPRD Thermal HAL";
        return 1;
    }
    joinRpcThreadpool();
    return 1;
}
