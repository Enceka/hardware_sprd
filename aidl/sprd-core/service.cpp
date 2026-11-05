/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.sprd.hardware-aidl-service.stub"

#include <android-base/logging.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android/binder_status.h>
#include <cstdlib>
#include <string>

#include <aidl/vendor/sprd/hardware/aprd/BnAprdInfoSync.h>
#include <aidl/vendor/sprd/hardware/cplog_svc/BnLogTransportInductor.h>
#include <aidl/vendor/sprd/hardware/cplog_svc/BnSysLogControl.h>
#include <aidl/vendor/sprd/hardware/enhance/BnEnhance.h>
#include <aidl/vendor/sprd/hardware/fingerprintmmi/BnFingerprintmmi.h>
#include <aidl/vendor/sprd/hardware/hdcp/BnHDCP.h>
#include <aidl/vendor/sprd/hardware/ifaa/BnIfaa.h>
#include <aidl/vendor/sprd/hardware/log/BnLogControl.h>
#include <aidl/vendor/sprd/hardware/network/BnNetworkControl.h>
#include <aidl/vendor/sprd/hardware/soter/BnSoter.h>
#include <aidl/vendor/sprd/hardware/trusty/BnTrustyClientProvider.h>
#include <aidl/vendor/sprd/hardware/tui/BnTui.h>

using ::aidl::vendor::sprd::hardware::aprd::BnAprdInfoSync;
using ::aidl::vendor::sprd::hardware::cplog_svc::BnLogTransportInductor;
using ::aidl::vendor::sprd::hardware::cplog_svc::BnSysLogControl;
using ::aidl::vendor::sprd::hardware::enhance::BnEnhance;
using ::aidl::vendor::sprd::hardware::fingerprintmmi::BnFingerprintmmi;
using ::aidl::vendor::sprd::hardware::hdcp::BnHDCP;
using ::aidl::vendor::sprd::hardware::ifaa::BnIfaa;
using ::aidl::vendor::sprd::hardware::log::BnLogControl;
using ::aidl::vendor::sprd::hardware::network::BnNetworkControl;
using ::aidl::vendor::sprd::hardware::soter::BnSoter;
using ::aidl::vendor::sprd::hardware::trusty::BnTrustyClientProvider;
using ::aidl::vendor::sprd::hardware::tui::BnTui;

class Stub_vendor_sprd_hardware_aprd_IAprdInfoSync : public BnAprdInfoSync {};
class Stub_vendor_sprd_hardware_cplog_svc_ISysLogControl : public BnSysLogControl {};
class Stub_vendor_sprd_hardware_cplog_svc_ILogTransportInductor : public BnLogTransportInductor {};
class Stub_vendor_sprd_hardware_enhance_IEnhance : public BnEnhance {};
class Stub_vendor_sprd_hardware_fingerprintmmi_IFingerprintmmi : public BnFingerprintmmi {};
class Stub_vendor_sprd_hardware_hdcp_IHDCP : public BnHDCP {};
class Stub_vendor_sprd_hardware_ifaa_IIfaa : public BnIfaa {};
class Stub_vendor_sprd_hardware_log_ILogControl : public BnLogControl {};
class Stub_vendor_sprd_hardware_network_INetworkControl : public BnNetworkControl {};
class Stub_vendor_sprd_hardware_soter_ISoter : public BnSoter {};
class Stub_vendor_sprd_hardware_trusty_ITrustyClientProvider : public BnTrustyClientProvider {};
class Stub_vendor_sprd_hardware_tui_ITui : public BnTui {};

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_aprd_IAprdInfoSync>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_aprd_IAprdInfoSync::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_cplog_svc_ISysLogControl>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_cplog_svc_ISysLogControl::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_cplog_svc_ILogTransportInductor>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_cplog_svc_ILogTransportInductor::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_enhance_IEnhance>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_enhance_IEnhance::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_fingerprintmmi_IFingerprintmmi>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_fingerprintmmi_IFingerprintmmi::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_hdcp_IHDCP>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_hdcp_IHDCP::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_ifaa_IIfaa>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_ifaa_IIfaa::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_log_ILogControl>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_log_ILogControl::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_network_INetworkControl>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_network_INetworkControl::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_soter_ISoter>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_soter_ISoter::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_trusty_ITrustyClientProvider>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_trusty_ITrustyClientProvider::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_sprd_hardware_tui_ITui>();
        const std::string instance = std::string(Stub_vendor_sprd_hardware_tui_ITui::descriptor) + "/default";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
