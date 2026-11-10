// SPDX-License-Identifier: Apache-2.0
#include "Lights.h"

#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <cstdlib>

using aidl::android::hardware::light::Lights;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    auto service = ndk::SharedRefBase::make<Lights>();
    const std::string instance = std::string(Lights::descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(service->asBinder().get(), instance.c_str()), STATUS_OK);
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;
}
