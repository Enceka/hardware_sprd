/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "vendor.unisoc.hardware.radio-service.stub"

#include <android-base/logging.h>
#include <android/binder_interface_utils.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <android/binder_status.h>
#include <cstdlib>
#include <string>

#include <aidl/vendor/unisoc/hardware/radio/BnExtRadio.h>
#include <aidl/vendor/unisoc/hardware/radio/data/BnExtRadioData.h>
#include <aidl/vendor/unisoc/hardware/radio/ims/BnImsRadio.h>
#include <aidl/vendor/unisoc/hardware/radio/lite/BnLiteRadio.h>
#include <aidl/vendor/unisoc/hardware/radio/messaging/BnExtRadioMessaging.h>
#include <aidl/vendor/unisoc/hardware/radio/modem/BnExtRadioModem.h>
#include <aidl/vendor/unisoc/hardware/radio/network/BnExtRadioNetwork.h>
#include <aidl/vendor/unisoc/hardware/radio/sim/BnExtRadioSim.h>
#include <aidl/vendor/unisoc/hardware/radio/voice/BnExtRadioVoice.h>

using ::aidl::vendor::unisoc::hardware::radio::BnExtRadio;
using ::aidl::vendor::unisoc::hardware::radio::data::BnExtRadioData;
using ::aidl::vendor::unisoc::hardware::radio::ims::BnImsRadio;
using ::aidl::vendor::unisoc::hardware::radio::lite::BnLiteRadio;
using ::aidl::vendor::unisoc::hardware::radio::messaging::BnExtRadioMessaging;
using ::aidl::vendor::unisoc::hardware::radio::modem::BnExtRadioModem;
using ::aidl::vendor::unisoc::hardware::radio::network::BnExtRadioNetwork;
using ::aidl::vendor::unisoc::hardware::radio::sim::BnExtRadioSim;
using ::aidl::vendor::unisoc::hardware::radio::voice::BnExtRadioVoice;

class Stub_vendor_unisoc_hardware_radio_IExtRadio : public BnExtRadio {};
class Stub_vendor_unisoc_hardware_radio_data_IExtRadioData : public BnExtRadioData {};
class Stub_vendor_unisoc_hardware_radio_ims_IImsRadio : public BnImsRadio {};
class Stub_vendor_unisoc_hardware_radio_lite_ILiteRadio : public BnLiteRadio {};
class Stub_vendor_unisoc_hardware_radio_messaging_IExtRadioMessaging : public BnExtRadioMessaging {};
class Stub_vendor_unisoc_hardware_radio_modem_IExtRadioModem : public BnExtRadioModem {};
class Stub_vendor_unisoc_hardware_radio_network_IExtRadioNetwork : public BnExtRadioNetwork {};
class Stub_vendor_unisoc_hardware_radio_sim_IExtRadioSim : public BnExtRadioSim {};
class Stub_vendor_unisoc_hardware_radio_voice_IExtRadioVoice : public BnExtRadioVoice {};

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(0);

    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_IExtRadio>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_IExtRadio::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_IExtRadio>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_IExtRadio::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_data_IExtRadioData>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_data_IExtRadioData::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_data_IExtRadioData>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_data_IExtRadioData::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_ims_IImsRadio>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_ims_IImsRadio::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_ims_IImsRadio>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_ims_IImsRadio::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_lite_ILiteRadio>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_lite_ILiteRadio::descriptor) + "/liteservice1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_lite_ILiteRadio>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_lite_ILiteRadio::descriptor) + "/liteservice2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_messaging_IExtRadioMessaging>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_messaging_IExtRadioMessaging::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_messaging_IExtRadioMessaging>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_messaging_IExtRadioMessaging::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_modem_IExtRadioModem>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_modem_IExtRadioModem::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_modem_IExtRadioModem>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_modem_IExtRadioModem::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_network_IExtRadioNetwork>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_network_IExtRadioNetwork::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_network_IExtRadioNetwork>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_network_IExtRadioNetwork::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_sim_IExtRadioSim>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_sim_IExtRadioSim::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_sim_IExtRadioSim>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_sim_IExtRadioSim::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_voice_IExtRadioVoice>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_voice_IExtRadioVoice::descriptor) + "/slot1";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }
    {
        auto hal = ndk::SharedRefBase::make<Stub_vendor_unisoc_hardware_radio_voice_IExtRadioVoice>();
        const std::string instance = std::string(Stub_vendor_unisoc_hardware_radio_voice_IExtRadioVoice::descriptor) + "/slot2";
        binder_status_t status =
                AServiceManager_addService(hal->asBinder().get(), instance.c_str());
        CHECK_EQ(status, STATUS_OK) << instance;
    }

    ABinderProcess_joinThreadPool();
    return EXIT_FAILURE;  // should not reach
}
