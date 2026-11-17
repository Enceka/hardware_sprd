#pragma once

#include <aidl/android/hardware/usb/BnUsb.h>
#include <android-base/unique_fd.h>

#include <mutex>
#include <thread>
#include <vector>

namespace aidl::android::hardware::usb {

class Usb final : public BnUsb {
   public:
    Usb();
    ~Usb() override;
    ndk::ScopedAStatus setCallback(const std::shared_ptr<IUsbCallback>& callback) override;
    ndk::ScopedAStatus queryPortStatus(int64_t transactionId) override;
    ndk::ScopedAStatus switchRole(const std::string& portName, const PortRole& role,
                                  int64_t transactionId) override;
    ndk::ScopedAStatus enableUsbData(const std::string& portName, bool enable,
                                     int64_t transactionId) override;
    ndk::ScopedAStatus enableUsbDataWhileDocked(const std::string& portName,
                                                int64_t transactionId) override;
    ndk::ScopedAStatus enableContaminantPresenceDetection(const std::string& portName, bool enable,
                                                          int64_t transactionId) override;
    ndk::ScopedAStatus limitPowerTransfer(const std::string& portName, bool limit,
                                          int64_t transactionId) override;
    ndk::ScopedAStatus resetUsbPort(const std::string& portName, int64_t transactionId) override;

   private:
    std::shared_ptr<IUsbCallback> callback();
    Status validatePort(const std::string& portName);
    Status readStatus(std::vector<PortStatus>* ports);
    void publish(const std::shared_ptr<IUsbCallback>& callback);
    void monitor();
    Status changeRole(const std::string& portName, const PortRole& role);
    std::mutex mCallbackMutex;
    std::mutex mNodeMutex;
    std::shared_ptr<IUsbCallback> mCallback;
    ::android::base::unique_fd mStop;
    ::android::base::unique_fd mUevent;
    std::thread mMonitor;
};

}
