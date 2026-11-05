/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.sprd.hardware-service.stub"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include <vendor/sprd/hardware/connmgr/1.0/IConnmgr.h>
#include <vendor/sprd/hardware/connmgr/1.0/IConnmgrCallback.h>
#include <vendor/sprd/hardware/cplog_svc/1.0/ILogTransportInductor.h>
#include <vendor/sprd/hardware/cplog_svc/1.0/ISubsysLogTransport.h>
#include <vendor/sprd/hardware/face/1.0/IExtBiometricsFace.h>
#include <vendor/sprd/hardware/gnss/1.0/IUnisocAGnssRil.h>
#include <vendor/sprd/hardware/gnss/1.0/IUnisocGnss.h>
#include <vendor/sprd/hardware/gnss/1.0/IUnisocGnssConfiguration.h>
#include <vendor/sprd/hardware/gnss/1.0/IUnisocGnssPowerIndication.h>
#include <vendor/sprd/hardware/gnss/1.0/IUnisocGnssPowerIndicationCallback.h>
#include <vendor/sprd/hardware/hdcp/1.0/IHDCP.h>
#include <vendor/sprd/hardware/ifaa/1.0/IIfaa.h>
#include <vendor/sprd/hardware/log/1.0/ILogControl.h>
#include <vendor/sprd/hardware/network/1.0/INetworkControl.h>
#include <vendor/sprd/hardware/power/4.0/IPower.h>
#include <vendor/sprd/hardware/radio/1.1/IExtRadio.h>
#include <vendor/sprd/hardware/radio/ims/1.1/IImsRadio.h>
#include <vendor/sprd/hardware/radio/lite/1.0/ILiteRadio.h>
#include <vendor/sprd/hardware/thermal/1.0/IExtThermal.h>
#include <vendor/sprd/hardware/thermal/2.0/IExtThermal.h>
#include <vendor/sprd/hardware/trusty/1.0/ITrustyClientProvider.h>
#include <vendor/sprd/hardware/tui/1.0/ITui.h>
#include <vendor/sprd/hardware/vdsp/1.0/IVdspService.h>
#include <vendor/sprd/hardware/wcn/1.0/IWcnVendorInterface.h>
#include <vendor/sprd/hardware/wifi/hostapd/1.1/IHostapd.h>

namespace ns0 = ::vendor::sprd::hardware::connmgr::V1_0;
namespace ns1 = ::vendor::sprd::hardware::cplog_svc::V1_0;
namespace ns2 = ::vendor::sprd::hardware::gnss::V1_0;
namespace ns3 = ::vendor::sprd::hardware::hdcp::V1_0;
namespace ns4 = ::vendor::sprd::hardware::face::V1_0;
namespace ns5 = ::vendor::sprd::hardware::ifaa::V1_0;
namespace ns6 = ::vendor::sprd::hardware::log::V1_0;
namespace ns7 = ::vendor::sprd::hardware::network::V1_0;
namespace ns8 = ::vendor::sprd::hardware::power::V4_0;
namespace ns9 = ::vendor::sprd::hardware::radio::V1_1;
namespace ns10 = ::vendor::sprd::hardware::radio::ims::V1_1;
namespace ns11 = ::vendor::sprd::hardware::radio::lite::V1_0;
namespace ns12 = ::vendor::sprd::hardware::thermal::V1_0;
namespace ns13 = ::vendor::sprd::hardware::thermal::V2_0;
namespace ns14 = ::vendor::sprd::hardware::trusty::V1_0;
namespace ns15 = ::vendor::sprd::hardware::tui::V1_0;
namespace ns16 = ::vendor::sprd::hardware::vdsp::V1_0;
namespace ns17 = ::vendor::sprd::hardware::wcn::V1_0;
namespace ns18 = ::vendor::sprd::hardware::wifi::hostapd::V1_1;

using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

struct Stub_connmgr_1_0_IConnmgr : public ns0::IConnmgr {};
struct Stub_connmgr_1_0_IConnmgrCallback : public ns0::IConnmgrCallback {};
struct Stub_cplog_svc_1_0_ILogTransportInductor : public ns1::ILogTransportInductor {};
struct Stub_cplog_svc_1_0_ISubsysLogTransport : public ns1::ISubsysLogTransport {};
struct Stub_gnss_1_0_IUnisocGnss : public ns2::IUnisocGnss {};
struct Stub_gnss_1_0_IUnisocAGnssRil : public ns2::IUnisocAGnssRil {};
struct Stub_gnss_1_0_IUnisocGnssConfiguration : public ns2::IUnisocGnssConfiguration {};
struct Stub_gnss_1_0_IUnisocGnssPowerIndication : public ns2::IUnisocGnssPowerIndication {};
struct Stub_gnss_1_0_IUnisocGnssPowerIndicationCallback : public ns2::IUnisocGnssPowerIndicationCallback {};
struct Stub_hdcp_1_0_IHDCP : public ns3::IHDCP {};
struct Stub_face_1_0_IExtBiometricsFace : public ns4::IExtBiometricsFace {};
struct Stub_ifaa_1_0_IIfaa : public ns5::IIfaa {};
struct Stub_log_1_0_ILogControl : public ns6::ILogControl {};
struct Stub_network_1_0_INetworkControl : public ns7::INetworkControl {};
struct Stub_power_4_0_IPower : public ns8::IPower {};
struct Stub_radio_1_1_IExtRadio : public ns9::IExtRadio {};
struct Stub_radio_ims_1_1_IImsRadio : public ns10::IImsRadio {};
struct Stub_radio_lite_1_0_ILiteRadio : public ns11::ILiteRadio {};
struct Stub_thermal_1_0_IExtThermal : public ns12::IExtThermal {};
struct Stub_thermal_2_0_IExtThermal : public ns13::IExtThermal {};
struct Stub_trusty_1_0_ITrustyClientProvider : public ns14::ITrustyClientProvider {};
struct Stub_tui_1_0_ITui : public ns15::ITui {};
struct Stub_vdsp_1_0_IVdspService : public ns16::IVdspService {};
struct Stub_wcn_1_0_IWcnVendorInterface : public ns17::IWcnVendorInterface {};
struct Stub_wifi_hostapd_1_1_IHostapd : public ns18::IHostapd {};

int main() {
    configureRpcThreadpool(1, true /* callerWillJoin */);

    {
        sp<ns0::IConnmgr> service = new Stub_connmgr_1_0_IConnmgr();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IConnmgr/default";
            return 1;
        }
    }
    {
        sp<ns1::ILogTransportInductor> service = new Stub_cplog_svc_1_0_ILogTransportInductor();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register ILogTransportInductor/default";
            return 1;
        }
    }
    {
        sp<ns2::IUnisocGnss> service = new Stub_gnss_1_0_IUnisocGnss();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IUnisocGnss/default";
            return 1;
        }
    }
    {
        sp<ns3::IHDCP> service = new Stub_hdcp_1_0_IHDCP();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IHDCP/default";
            return 1;
        }
    }
    {
        sp<ns4::IExtBiometricsFace> service = new Stub_face_1_0_IExtBiometricsFace();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IExtBiometricsFace/default";
            return 1;
        }
    }
    {
        sp<ns5::IIfaa> service = new Stub_ifaa_1_0_IIfaa();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IIfaa/default";
            return 1;
        }
    }
    {
        sp<ns6::ILogControl> service = new Stub_log_1_0_ILogControl();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register ILogControl/default";
            return 1;
        }
    }
    {
        sp<ns7::INetworkControl> service = new Stub_network_1_0_INetworkControl();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register INetworkControl/default";
            return 1;
        }
    }
    {
        sp<ns8::IPower> service = new Stub_power_4_0_IPower();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IPower/default";
            return 1;
        }
    }
    {
        sp<ns9::IExtRadio> service = new Stub_radio_1_1_IExtRadio();
        if (service->registerAsService("slot1") != android::OK) {
            LOG(ERROR) << "Failed to register IExtRadio/slot1";
            return 1;
        }
    }
    {
        sp<ns9::IExtRadio> service = new Stub_radio_1_1_IExtRadio();
        if (service->registerAsService("slot2") != android::OK) {
            LOG(ERROR) << "Failed to register IExtRadio/slot2";
            return 1;
        }
    }
    {
        sp<ns10::IImsRadio> service = new Stub_radio_ims_1_1_IImsRadio();
        if (service->registerAsService("slot1") != android::OK) {
            LOG(ERROR) << "Failed to register IImsRadio/slot1";
            return 1;
        }
    }
    {
        sp<ns10::IImsRadio> service = new Stub_radio_ims_1_1_IImsRadio();
        if (service->registerAsService("slot2") != android::OK) {
            LOG(ERROR) << "Failed to register IImsRadio/slot2";
            return 1;
        }
    }
    {
        sp<ns11::ILiteRadio> service = new Stub_radio_lite_1_0_ILiteRadio();
        if (service->registerAsService("liteservice1") != android::OK) {
            LOG(ERROR) << "Failed to register ILiteRadio/liteservice1";
            return 1;
        }
    }
    {
        sp<ns11::ILiteRadio> service = new Stub_radio_lite_1_0_ILiteRadio();
        if (service->registerAsService("liteservice2") != android::OK) {
            LOG(ERROR) << "Failed to register ILiteRadio/liteservice2";
            return 1;
        }
    }
    {
        sp<ns12::IExtThermal> service = new Stub_thermal_1_0_IExtThermal();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IExtThermal/default";
            return 1;
        }
    }
    {
        sp<ns13::IExtThermal> service = new Stub_thermal_2_0_IExtThermal();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IExtThermal/default";
            return 1;
        }
    }
    {
        sp<ns14::ITrustyClientProvider> service = new Stub_trusty_1_0_ITrustyClientProvider();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register ITrustyClientProvider/default";
            return 1;
        }
    }
    {
        sp<ns15::ITui> service = new Stub_tui_1_0_ITui();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register ITui/default";
            return 1;
        }
    }
    {
        sp<ns16::IVdspService> service = new Stub_vdsp_1_0_IVdspService();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IVdspService/default";
            return 1;
        }
    }
    {
        sp<ns17::IWcnVendorInterface> service = new Stub_wcn_1_0_IWcnVendorInterface();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IWcnVendorInterface/default";
            return 1;
        }
    }
    {
        sp<ns18::IHostapd> service = new Stub_wifi_hostapd_1_1_IHostapd();
        if (service->registerAsService("default") != android::OK) {
            LOG(ERROR) << "Failed to register IHostapd/default";
            return 1;
        }
    }

    joinRpcThreadpool();
    return 0;  // should never get here
}
