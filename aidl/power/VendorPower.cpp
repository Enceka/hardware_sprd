// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include "VendorPower.h"

#include <android-base/logging.h>
#include <android/binder_ibinder.h>
#include <android/binder_parcel_utils.h>
#include <android/binder_stability.h>
#include <android/binder_status.h>
#include <unistd.h>

namespace sprd::power {
namespace {
ndk::ScopedAStatus Invalid() { return ndk::ScopedAStatus::fromExceptionCode(EX_ILLEGAL_ARGUMENT); }
}  // namespace
VendorPower::VendorPower(std::shared_ptr<SceneManager> scenes) : mState(std::make_shared<State>()) {
    mState->scenes = std::move(scenes);
    mDeath = AIBinder_DeathRecipient_new(Died);
    AIBinder_DeathRecipient_setOnUnlinked(mDeath, Unlinked);
}
VendorPower::~VendorPower() {
    // Cookies own only weak State references. Death callbacks may finish after
    // unlink, and onUnlinked is the sole cookie deleter.
    auto state = std::move(mState);
    std::lock_guard lock(state->mutex);
    for (const auto& [key, client] : state->clients) {
        for (const auto& owner : client.owners) state->scenes->ReleaseOwner(owner);
        if (client.cookie) AIBinder_unlinkToDeath(client.binder.get(), mDeath, client.cookie);
    }
    AIBinder_DeathRecipient_delete(mDeath);
}
void VendorPower::Died(void* data) {
    auto cookie = static_cast<Cookie*>(data);
    if (auto state = cookie->state.lock()) {
        std::lock_guard lock(state->mutex);
        auto it = state->clients.find(cookie->key);
        if (it != state->clients.end() && it->second.cookie == cookie) {
            for (const auto& owner : it->second.owners) state->scenes->ReleaseOwner(owner);
            auto uid = cookie->key.first;
            auto tags = it->second.tags;
            state->clients.erase(it);
            for (const auto& tag : tags) {
                bool otherToken = false;
                for (const auto& [key, client] : state->clients)
                    if (key.first == uid && client.tags.count(tag)) otherToken = true;
                if (!otherToken) state->scenes->DropTag(uid, tag);
            }
        }
    }
}
void VendorPower::Unlinked(void* data) { delete static_cast<Cookie*>(data); }
ndk::ScopedAStatus VendorPower::Lease(const ndk::SpAIBinder& token, const std::string& tag,
                                      const std::string& scene, bool acquire) {
    if (!token.get() || tag.size() > 128 || scene.empty()) return Invalid();
    std::lock_guard lock(mState->mutex);
    auto key = std::make_pair(AIBinder_getCallingUid(), token.get());
    auto it = mState->clients.find(key);
    if (it == mState->clients.end()) {
        if (!acquire) return ndk::ScopedAStatus::ok();
        if (mState->clients.size() >= 1024) return Invalid();
        auto cookie = new Cookie{mState, key};
        // Insert before linking: a concurrent death callback takes the mutex
        // after this transaction and will see the complete lease state.
        Client client{token, "vendor:" + std::to_string(++mState->nextOwner), cookie, {}, {}};
        it = mState->clients.emplace(key, std::move(client)).first;
        auto status = AIBinder_linkToDeath(token.get(), mDeath, cookie);
        if (status != STATUS_OK) {
            // onUnlinked runs even for failed links. Local tokens are useful
            // for in-process clients and have the same service lifetime.
            it->second.cookie = nullptr;
            if (status != STATUS_INVALID_OPERATION) {
                mState->clients.erase(it);
                return ndk::ScopedAStatus::fromStatus(status);
            }
        }
    }
    // Leases are independent for each token/tag pair. Death releases every tag.
    auto owner = it->second.owner + ":" + tag;
    it->second.owners.insert(owner);
    it->second.tags.insert(tag);
    if (acquire) {
        if (!mState->scenes->Acquire(owner, scene)) return Invalid();
    } else
        mState->scenes->Release(owner, scene);
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus VendorPower::acquirePowerHintBySceneId(const ndk::SpAIBinder& token,
                                                          const std::string& tag, int32_t id) {
    return Lease(token, tag, mState->scenes->SceneName(id), true);
}
ndk::ScopedAStatus VendorPower::releasePowerHintBySceneId(const ndk::SpAIBinder& token,
                                                          const std::string& tag, int32_t id) {
    return Lease(token, tag, mState->scenes->SceneName(id), false);
}
ndk::ScopedAStatus VendorPower::acquirePowerHintBySceneName(const ndk::SpAIBinder& token,
                                                            const std::string& tag,
                                                            const std::string& name) {
    return Lease(token, tag, name, true);
}
ndk::ScopedAStatus VendorPower::releasePowerHintBySceneName(const ndk::SpAIBinder& token,
                                                            const std::string& tag,
                                                            const std::string& name) {
    return Lease(token, tag, name, false);
}
ndk::ScopedAStatus VendorPower::getSceneIdByName(const std::string& name, int32_t* id) {
    *id = mState->scenes->SceneId(name);
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus VendorPower::controlPowerHint(int32_t enabled) {
    mState->scenes->Control(enabled != 0);
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus VendorPower::registerPowerHintScene(const std::string& tag,
                                                       const std::vector<std::string>& config,
                                                       int32_t* id) {
    *id = mState->scenes->Register(AIBinder_getCallingUid(), tag, config);
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus VendorPower::unregisterPowerHintScene(const std::string& tag, int32_t id) {
    if (!mState->scenes->Unregister(AIBinder_getCallingUid(), tag, id)) return Invalid();
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus VendorPower::setVipTaskLevel(const std::string&, const std::string& type,
                                                int32_t pid, int32_t level) {
    if (!mState->scenes->Vip(type, pid, level)) return Invalid();
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus VendorPower::getInterfaceHash(std::string* hash) {
    // Wire-compatible with the nine-method V1 shipped on the E5. Keep the
    // separately named build interface from shadowing the OEM client library.
    *hash = "67a6288e6ac1c48aa2d0f1c882d182d74ad6f61b";
    return ndk::ScopedAStatus::ok();
}
ndk::ScopedAStatus VendorPower::getInterfaceVersion(int32_t* version) {
    *version = 1;
    return ndk::ScopedAStatus::ok();
}
ndk::SpAIBinder VendorPower::createBinder() {
    static auto clazz = ndk::ICInterface::defineClass(descriptor, Transact);
    ndk::SpAIBinder binder(AIBinder_new(clazz, static_cast<ndk::ICInterface*>(this)));
    AIBinder_markVintfStability(binder.get());
    return binder;
}
binder_status_t VendorPower::Transact(AIBinder* binder, transaction_code_t code, const AParcel* in,
                                      AParcel* out) {
    auto self = std::static_pointer_cast<VendorPower>(ndk::ICInterface::asInterface(binder));
    ndk::ScopedAStatus result = ndk::ScopedAStatus::ok();
    binder_status_t status;
    ndk::SpAIBinder token;
    std::string tag, name, hash;
    std::vector<std::string> config;
    int32_t value = 0, level = 0;
#define READ(value)                                   \
    do {                                              \
        status = ndk::AParcel_readData(in, &(value)); \
        if (status != STATUS_OK) return status;       \
    } while (0)
    switch (code) {
        case 1:
        case 2:
            READ(token);
            READ(tag);
            READ(value);
            result = code == 1 ? self->acquirePowerHintBySceneId(token, tag, value)
                               : self->releasePowerHintBySceneId(token, tag, value);
            break;
        case 3:
        case 4:
            READ(token);
            READ(tag);
            READ(name);
            result = code == 3 ? self->acquirePowerHintBySceneName(token, tag, name)
                               : self->releasePowerHintBySceneName(token, tag, name);
            break;
        case 5:
            READ(name);
            result = self->getSceneIdByName(name, &value);
            break;
        case 6:
            READ(value);
            result = self->controlPowerHint(value);
            break;
        case 7:
            READ(tag);
            READ(config);
            result = self->registerPowerHintScene(tag, config, &value);
            break;
        case 8:
            READ(tag);
            READ(value);
            result = self->unregisterPowerHintScene(tag, value);
            break;
        case 9:
            READ(tag);
            READ(name);
            READ(value);
            READ(level);
            result = self->setVipTaskLevel(tag, name, value, level);
            break;
        case 16777214:
            result = self->getInterfaceHash(&hash);
            break;
        case 16777215:
            result = self->getInterfaceVersion(&value);
            break;
        default:
            return STATUS_UNKNOWN_TRANSACTION;
    }
#undef READ
    // Transactions 1..4, 8 and 9 are oneway in the stock interface.
    if (code <= 4 || code == 8 || code == 9) {
        if (!result.isOk())
            LOG(WARNING) << "Vendor Power transaction " << code << ": " << result.getDescription();
        return STATUS_OK;
    }
    status = AParcel_writeStatusHeader(out, result.get());
    if (status != STATUS_OK || !result.isOk()) return status;
    if (code == 5 || code == 7 || code == 16777215) return ndk::AParcel_writeData(out, value);
    if (code == 16777214) return ndk::AParcel_writeData(out, hash);
    return STATUS_OK;
}

}  // namespace sprd::power
