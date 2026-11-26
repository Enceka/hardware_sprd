// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <aidl/vendor/unisoc/hardware/power/IVendorPower.h>

#include <map>
#include <memory>
#include <mutex>

#include "SceneManager.h"

namespace sprd::power {
class VendorPower final
    : public ndk::BnCInterface<aidl::vendor::unisoc::hardware::power::IVendorPower> {
   public:
    explicit VendorPower(std::shared_ptr<SceneManager> scenes);
    ~VendorPower() override;
    ndk::ScopedAStatus acquirePowerHintBySceneId(const ndk::SpAIBinder&, const std::string&,
                                                 int32_t) override;
    ndk::ScopedAStatus releasePowerHintBySceneId(const ndk::SpAIBinder&, const std::string&,
                                                 int32_t) override;
    ndk::ScopedAStatus acquirePowerHintBySceneName(const ndk::SpAIBinder&, const std::string&,
                                                   const std::string&) override;
    ndk::ScopedAStatus releasePowerHintBySceneName(const ndk::SpAIBinder&, const std::string&,
                                                   const std::string&) override;
    ndk::ScopedAStatus getSceneIdByName(const std::string&, int32_t*) override;
    ndk::ScopedAStatus controlPowerHint(int32_t) override;
    ndk::ScopedAStatus registerPowerHintScene(const std::string&, const std::vector<std::string>&,
                                              int32_t*) override;
    ndk::ScopedAStatus unregisterPowerHintScene(const std::string&, int32_t) override;
    ndk::ScopedAStatus setVipTaskLevel(const std::string&, const std::string&, int32_t,
                                       int32_t) override;
    ndk::ScopedAStatus getInterfaceVersion(int32_t*);
    ndk::ScopedAStatus getInterfaceHash(std::string*);

   private:
    ndk::SpAIBinder createBinder() override;
    static binder_status_t Transact(AIBinder*, transaction_code_t, const AParcel*, AParcel*);
    struct Cookie;
    struct Client {
        ndk::SpAIBinder binder;
        std::string owner;
        Cookie* cookie;
        std::set<std::string> owners, tags;
    };
    struct State {
        std::mutex mutex;
        std::shared_ptr<SceneManager> scenes;
        std::map<std::pair<uid_t, AIBinder*>, Client> clients;
        uint64_t nextOwner = 0;
    };
    struct Cookie {
        std::weak_ptr<State> state;
        std::pair<uid_t, AIBinder*> key;
    };
    static void Died(void* cookie);
    static void Unlinked(void* cookie);
    ndk::ScopedAStatus Lease(const ndk::SpAIBinder&, const std::string&, const std::string&, bool);
    std::shared_ptr<State> mState;
    AIBinder_DeathRecipient* mDeath;
};
}  // namespace sprd::power
