#include <android-base/logging.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>

#include <cstdlib>

#include "Usb.h"

using aidl::android::hardware::usb::Usb;

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);
    auto service = ndk::SharedRefBase::make<Usb>();
    const std::string instance = std::string(Usb::descriptor) + "/default";
    CHECK_EQ(AServiceManager_addService(service->asBinder().get(), instance.c_str()), STATUS_OK);
    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;
}
