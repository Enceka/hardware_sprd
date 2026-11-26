// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include "SceneManager.h"

#include <tinyxml2.h>
#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <sstream>

namespace sprd::power {
using namespace std::chrono;
using tinyxml2::XMLElement;
namespace {
std::string Attr(const XMLElement* e, const char* key) {
    auto value = e->Attribute(key);
    return value ? value : "";
}
// Compare numeric vectors too (e.g. sched_cap_margin_up), with latest value
// as the tie-break for nonnumeric configuration values.
bool Less(const std::string& a, const std::string& b) {
    std::istringstream sa(a), sb(b);
    long long va, vb;
    while (sa >> va && sb >> vb) {
        if (va != vb) return va < vb;
    }
    return a < b;
}
}  // namespace
SceneManager::SceneManager(Read read, Write write)
    : mRead(std::move(read)), mWrite(std::move(write)) {}
SceneManager::~SceneManager() {
    {
        std::lock_guard lock(mMutex);
        mStop = true;
        mCv.notify_all();
    }
    if (mThread.joinable()) mThread.join();
    std::lock_guard lock(mMutex);
    mRequests.clear();
    ApplyLocked();
}
bool SceneManager::Expand(const std::string& path, const std::string& file,
                          const std::string& value, Actions* actions) const {
    if (path == "subsys") {
        auto it = mSubsystems.find(file + "/" + value);
        if (it == mSubsystems.end()) return false;
        actions->insert(it->second.begin(), it->second.end());
        return true;
    }
    auto key = path + "/" + file;
    if (!mNodes.count(key)) return false;
    (*actions)[key] = value;
    return true;
}
bool SceneManager::Load(const std::string& directory) {
    std::lock_guard lock(mMutex);
    if (mThread.joinable()) return false;
    tinyxml2::XMLDocument resources, scenes;
    if (resources.LoadFile((directory + "/power_resource_file_info.xml").c_str()) != 0 ||
        scenes.LoadFile((directory + "/power_scene_config.xml").c_str()) != 0)
        return false;
    auto root = resources.FirstChildElement("resources");
    auto power = scenes.FirstChildElement("power");
    if (!root || !power) return false;
    for (auto e = root->FirstChildElement("file"); e; e = e->NextSiblingElement("file")) {
        Node node;
        node.path = Attr(e, "path") + "/" + Attr(e, "file");
        node.ddr = Attr(e, "file") == "scene_boost_dfs";
        bool supported = true;
        for (auto a = e->FirstChildElement("attr"); a; a = a->NextSiblingElement("attr")) {
            auto name = Attr(a, "name"), value = Attr(a, "value");
            if (name == "comp_func") node.minimum = value.find("descend") != std::string::npos;
            if (name == "def_value") {
                node.baseline = value;
                node.configuredDefault = true;
            }
            if (name == "set_func" && value != "common_set" && value != "devfreq_ddr_set")
                supported = false;
        }
        if (Attr(e, "path") == "subsys") continue;
        if (!supported) continue;
        mResourceNames[Attr(e, "name")] = node.path;
        mNodes[node.path] = node;
    }
    for (auto e = root->FirstChildElement("subsys"); e; e = e->NextSiblingElement("subsys")) {
        auto name = Attr(e, "name");
        for (auto inode = e->FirstChildElement("inode"); inode;
             inode = inode->NextSiblingElement("inode")) {
            Node node;
            node.path = Attr(inode, "path") + "/" + Attr(inode, "file");
            mNodes.emplace(node.path, node);
            mResourceNames[Attr(inode, "name")] = node.path;
        }
        for (auto conf = e->FirstChildElement("conf"); conf;
             conf = conf->NextSiblingElement("conf")) {
            auto& actions = mSubsystems[name + "/" + Attr(conf, "name")];
            for (auto a = conf->FirstChildElement("set"); a; a = a->NextSiblingElement("set"))
                if (!Expand(Attr(a, "path"), Attr(a, "file"), Attr(a, "value"), &actions))
                    return false;
        }
    }
    // Stock E5 alternative modes contain no policies. Only load the normal
    // scene definitions; mode hints with no actions are not advertised.
    auto normal = power->FirstChildElement("mode");
    while (normal && Attr(normal, "name") != "normal") normal = normal->NextSiblingElement("mode");
    if (!normal) return false;
    for (auto e = normal->FirstChildElement("scene"); e; e = e->NextSiblingElement("scene")) {
        Actions actions;
        for (auto a = e->FirstChildElement("set"); a; a = a->NextSiblingElement("set")) {
            if (!Expand(Attr(a, "path"), Attr(a, "file"), Attr(a, "value"), &actions)) return false;
        }
        mScenes[Attr(e, "name")] = std::move(actions);
    }
    std::string ids;
    if (!mRead(directory + "/power_scene_id_define.txt", &ids)) return false;
    std::istringstream lines(ids);
    std::string line;
    while (std::getline(lines, line)) {
        std::istringstream fields(line);
        uint32_t id, sub;
        std::string name;
        if (fields >> std::hex >> id >> sub >> name && sub == 0) mIds.emplace(name, id);
    }
    mThread = std::thread(&SceneManager::Loop, this);
    return true;
}
std::string SceneManager::Resolve(const std::string& hint) const {
    if (hint == "INTERACTION") return "interaction_fling";
    if (hint == "CAMERA_LAUNCH" || hint == "CAMERA_SHOT") return "camera_high_perf";
    if (hint == "DISPLAY_UPDATE_IMMINENT") return "screen_rotation";
    std::string name = hint;
    std::transform(name.begin(), name.end(), name.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return name;
}
bool SceneManager::IsHintSupported(const std::string& hint) const {
    std::lock_guard lock(mMutex);
    auto it = mScenes.find(Resolve(hint));
    return it != mScenes.end() && !it->second.empty();
}
bool SceneManager::RequestLocked(const std::string& owner, const std::string& scene,
                                 milliseconds duration, bool replace) {
    if (!mEnabled || !mScenes.count(scene)) return false;
    auto key = std::make_pair(owner, scene);
    auto it = mRequests.find(key);
    if (it == mRequests.end() && mRequests.size() >= 4096) return false;
    auto& request = mRequests[key];
    if (!replace && !request.scene.empty()) {
        if (request.count >= 4096) return false;
        ++request.count;
    }
    request.scene = scene;
    request.expiry =
        duration.count() > 0 ? steady_clock::now() + duration : steady_clock::time_point::max();
    ApplyLocked();
    mCv.notify_all();
    return true;
}
bool SceneManager::DoHint(const std::string& hint, milliseconds duration) {
    std::lock_guard lock(mMutex);
    if (!duration.count()) {
        if (hint == "INTERACTION") duration = milliseconds(150);
        if (hint == "CAMERA_LAUNCH" || hint == "CAMERA_SHOT") duration = milliseconds(1000);
        if (hint == "DISPLAY_UPDATE_IMMINENT") duration = milliseconds(150);
    }
    return RequestLocked("standard:" + hint, Resolve(hint), duration, true);
}
void SceneManager::EndHint(const std::string& hint) { ReleaseOwner("standard:" + hint); }
bool SceneManager::Acquire(const std::string& owner, const std::string& scene) {
    std::lock_guard lock(mMutex);
    return RequestLocked(owner, scene, {}, false);
}
void SceneManager::Release(const std::string& owner, const std::string& scene) {
    std::lock_guard lock(mMutex);
    auto it = mRequests.find({owner, scene});
    if (it != mRequests.end() && --it->second.count == 0) mRequests.erase(it);
    ApplyLocked();
    mCv.notify_all();
}
void SceneManager::ReleaseOwner(const std::string& owner) {
    std::lock_guard lock(mMutex);
    for (auto it = mRequests.begin(); it != mRequests.end();) {
        if (it->first.first == owner)
            it = mRequests.erase(it);
        else
            ++it;
    }
    ApplyLocked();
    mCv.notify_all();
}
int SceneManager::SceneId(const std::string& name) const {
    std::lock_guard lock(mMutex);
    auto it = mIds.find(name);
    return it == mIds.end() ? 0 : it->second;
}
std::string SceneManager::SceneName(int id) const {
    std::lock_guard lock(mMutex);
    for (const auto& [name, value] : mIds)
        if (value == id) return name;
    return {};
}
int SceneManager::Register(int uid, const std::string& tag, const std::vector<std::string>& pairs) {
    std::lock_guard lock(mMutex);
    if (pairs.empty() || pairs.size() % 2 || pairs.size() > 50 || tag.empty() || tag.size() > 128)
        return 0;
    Actions actions;
    for (size_t i = 0; i < pairs.size(); i += 2) {
        auto it = mResourceNames.find(pairs[i]);
        if (it == mResourceNames.end() || pairs[i + 1].empty() || pairs[i + 1].size() > 128)
            return 0;
        // Values must be numeric vectors or DDR symbolic extrema. No paths
        // supplied by callers are opened; resources come only from OEM XML.
        auto value = pairs[i + 1];
        if (value != "max" && value != "min" &&
            value.find_first_not_of("0123456789 +-\t") != std::string::npos)
            return 0;
        actions[it->second] = value;
    }
    for (const auto& [id, entry] : mDynamic) {
        if (entry.uid == uid && entry.tag == tag) {
            mScenes[entry.name] = std::move(actions);
            ApplyLocked();
            return id;
        }
    }
    for (int id = 0x7ffff000; id <= 0x7ffff004; ++id) {
        if (mDynamic.count(id)) continue;
        auto name = "dynamic_" + std::to_string(id - 0x7ffff000 + 1);
        mDynamic[id] = {uid, tag, name};
        mIds[name] = id;
        mScenes[name] = std::move(actions);
        return id;
    }
    return 0;
}
void SceneManager::DropTag(int uid, const std::string& tag) {
    std::vector<std::pair<std::string, int>> registrations;
    {
        std::lock_guard lock(mMutex);
        for (const auto& [id, entry] : mDynamic)
            if (entry.uid == uid && entry.tag == tag) registrations.emplace_back(entry.tag, id);
    }
    for (const auto& [tag, id] : registrations) Unregister(uid, tag, id);
}
bool SceneManager::Unregister(int uid, const std::string& tag, int id) {
    std::lock_guard lock(mMutex);
    auto it = mDynamic.find(id);
    if (it == mDynamic.end() || it->second.uid != uid || it->second.tag != tag) return false;
    auto name = it->second.name;
    for (auto r = mRequests.begin(); r != mRequests.end();) {
        if (r->second.scene == name)
            r = mRequests.erase(r);
        else
            ++r;
    }
    mScenes.erase(name);
    mIds.erase(name);
    mDynamic.erase(it);
    ApplyLocked();
    mCv.notify_all();
    return true;
}
void SceneManager::Control(bool enabled) {
    std::lock_guard lock(mMutex);
    mEnabled = enabled;
    if (!enabled) {
        mRequests.clear();
        ApplyLocked();
        mCv.notify_all();
    }
}
bool SceneManager::Vip(const std::string& type, int pid, int level) {
    static const std::map<std::string, std::string> files = {{"audio", "sched_task_audio"},
                                                             {"ui", "sched_task_ui"},
                                                             {"camera", "sched_task_ll_camera"},
                                                             {"animator", "sched_task_animator"}};
    auto it = files.find(type);
    if (it == files.end() || pid <= 0 || level < 0) return false;
    return mWrite("/proc/sys/walt/" + it->second,
                  std::to_string(pid) + " " + std::to_string(level));
}
void SceneManager::ApplyLocked() {
    Actions targets;
    for (const auto& [key, request] : mRequests) {
        for (const auto& [path, value] : mScenes.at(request.scene)) {
            auto it = targets.find(path);
            if (it == targets.end() ||
                (mNodes.at(path).minimum ? Less(value, it->second) : Less(it->second, value)))
                targets[path] = value;
        }
    }
    for (auto& [path, node] : mNodes) {
        if (node.ddr) continue;  // DDR is handled by its vote protocol, not ordinary file writes.
        auto it = targets.find(path);
        if (!node.captured && it != targets.end()) {
            if (!node.configuredDefault && !mRead(path, &node.baseline)) continue;
            node.captured = true;
        }
        if (!node.captured) continue;
        const auto& desired = it == targets.end() ? node.baseline : it->second;
        if (desired != node.applied && mWrite(path, desired)) node.applied = desired;
    }
}
void SceneManager::Loop() {
    std::unique_lock lock(mMutex);
    while (!mStop) {
        auto deadline = steady_clock::time_point::max();
        auto now = steady_clock::now();
        bool changed = false;
        for (auto it = mRequests.begin(); it != mRequests.end();) {
            if (it->second.expiry <= now) {
                it = mRequests.erase(it);
                changed = true;
            } else {
                deadline = std::min(deadline, it->second.expiry);
                ++it;
            }
        }
        if (changed) ApplyLocked();
        if (deadline == steady_clock::time_point::max())
            mCv.wait(lock);
        else
            mCv.wait_until(lock, deadline);
    }
}
void SceneManager::DumpToFd(int fd) {
    std::lock_guard lock(mMutex);
    dprintf(fd, "SPRD scene engine: enabled=%d requests=%zu dynamic=%zu\n", mEnabled,
            mRequests.size(), mDynamic.size());
    for (const auto& [key, request] : mRequests)
        dprintf(fd, "%s scene=%s references=%u\n", key.first.c_str(), request.scene.c_str(),
                request.count);
    for (const auto& [path, node] : mNodes)
        if (node.captured)
            dprintf(fd, "%s applied=%s baseline=%s\n", path.c_str(), node.applied.c_str(),
                    node.baseline.c_str());
}
}  // namespace sprd::power
