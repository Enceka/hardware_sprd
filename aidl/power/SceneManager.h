// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <chrono>
#include <condition_variable>
#include <functional>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <thread>
#include <vector>

namespace sprd::power {
class SceneManager {
   public:
    using Read = std::function<bool(const std::string&, std::string*)>;
    using Write = std::function<bool(const std::string&, const std::string&)>;
    SceneManager(Read read, Write write);
    ~SceneManager();
    bool Load(const std::string& directory);
    bool IsHintSupported(const std::string& hint) const;
    bool DoHint(const std::string& hint, std::chrono::milliseconds duration = {});
    void EndHint(const std::string& hint);
    bool Acquire(const std::string& owner, const std::string& scene);
    void Release(const std::string& owner, const std::string& scene);
    void ReleaseOwner(const std::string& owner);
    int SceneId(const std::string& name) const;
    std::string SceneName(int id) const;
    int Register(int uid, const std::string& tag, const std::vector<std::string>& pairs);
    void DropTag(int uid, const std::string& tag);
    bool Unregister(int uid, const std::string& tag, int id);
    void Control(bool enabled);
    bool Vip(const std::string& type, int pid, int level);
    void DumpToFd(int fd);

   private:
    struct Node {
        std::string path, baseline, applied;
        bool minimum = false;
        bool captured = false;
        bool configuredDefault = false;
        bool ddr = false;
    };
    using Actions = std::map<std::string, std::string>;
    struct Request {
        std::string scene;
        unsigned count = 1;
        std::chrono::steady_clock::time_point expiry;
    };
    struct Dynamic {
        int uid;
        std::string tag, name;
    };
    std::string Resolve(const std::string& hint) const;
    bool RequestLocked(const std::string& owner, const std::string& scene,
                       std::chrono::milliseconds duration, bool replace);
    void ApplyLocked();
    void Loop();
    bool Expand(const std::string& path, const std::string& file, const std::string& value,
                Actions* actions) const;
    Read mRead;
    Write mWrite;
    mutable std::mutex mMutex;
    std::condition_variable mCv;
    bool mStop = false, mEnabled = true;
    std::thread mThread;
    std::map<std::string, Node> mNodes;
    std::map<std::string, std::string> mResourceNames;
    std::map<std::string, Actions> mSubsystems;
    std::map<std::string, Actions> mScenes;
    std::map<std::string, int> mIds;
    std::map<int, Dynamic> mDynamic;
    std::map<std::pair<std::string, std::string>, Request> mRequests;
};
}  // namespace sprd::power
