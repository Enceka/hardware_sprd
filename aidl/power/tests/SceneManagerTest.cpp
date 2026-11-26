// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <thread>

#include "../SceneManager.h"

int main(int argc, char** argv) {
    assert(argc == 2 || argc == 3);
    std::mutex mutex;
    std::map<std::string, std::string> files{{"/fake/cpu/scaling_min_freq", "100"},
                                             {"/fake/cpu/scaling_max_freq", "300"},
                                             {"/fake/ddr/ddrinfo_freq_table", "200 800 400 255 0"}};
    std::vector<std::pair<std::string, std::string>> writes;
    std::string failValue;
    auto read = [&](const std::string& path, std::string* value) {
        std::lock_guard lock(mutex);
        if (files.count(path)) {
            *value = files[path];
            return true;
        }
        std::ifstream input(path);
        if (!input) return false;
        std::ostringstream contents;
        contents << input.rdbuf();
        *value = contents.str();
        return true;
    };
    auto write = [&](const std::string& path, const std::string& value) {
        std::lock_guard lock(mutex);
        if (value == failValue) return false;
        if (path == "/fake/cpu/scaling_min_freq" &&
            std::stoi(value) > std::stoi(files.at("/fake/cpu/scaling_max_freq")))
            return false;
        if (path == "/fake/cpu/scaling_max_freq" &&
            std::stoi(value) < std::stoi(files.at("/fake/cpu/scaling_min_freq")))
            return false;
        files[path] = value;
        writes.emplace_back(path, value);
        return true;
    };
    auto current = [&](const std::string& path) {
        std::lock_guard lock(mutex);
        return files.at(path);
    };
    using namespace std::chrono_literals;
    sprd::power::SceneManager manager(read, write);
    assert(manager.Load(argv[1]));
    if (argc == 3) {
        std::cout << "PASS: complete OEM XML loads\n";
        return 0;
    }
    assert(manager.SceneId("launch") == 6);
    assert(manager.SceneName(6) == "launch");
    assert(!manager.Acquire("a", "missing"));
    assert(manager.Acquire("limits", "high"));
    assert(current("/fake/cpu/scaling_min_freq") == "400");
    assert(current("/fake/cpu/scaling_max_freq") == "400");
    manager.ReleaseOwner("limits");
    assert(current("/fake/cpu/scaling_min_freq") == "100");
    assert(current("/fake/cpu/scaling_max_freq") == "300");
    assert(manager.Acquire("a", "launch"));
    assert(manager.Acquire("b", "interaction_fling"));
    assert(current("/fake/cpu/scaling_min_freq") == "200");
    manager.ReleaseOwner("a");
    assert(current("/fake/cpu/scaling_min_freq") == "150");
    manager.ReleaseOwner("b");
    assert(current("/fake/cpu/scaling_min_freq") == "100");
    assert(manager.Acquire("a", "launch"));
    assert(manager.Acquire("a", "launch"));
    manager.Release("a", "launch");
    assert(current("/fake/cpu/scaling_min_freq") == "200");
    manager.Release("a", "launch");
    assert(current("/fake/cpu/scaling_min_freq") == "100");
    manager.DoHint("INTERACTION", 20ms);
    manager.Acquire("private", "launch");
    std::this_thread::sleep_for(50ms);
    assert(current("/fake/cpu/scaling_min_freq") == "200");
    manager.ReleaseOwner("private");
    assert(current("/fake/cpu/scaling_min_freq") == "100");
    int id = manager.Register(1000, "camera", {"cpu_min", "180"});
    assert(id != 0);
    assert(manager.Acquire("camera", manager.SceneName(id)));
    assert(current("/fake/cpu/scaling_min_freq") == "180");
    assert(!manager.Unregister(1001, "camera", id));
    assert(!manager.Unregister(1000, "other", id));
    assert(manager.Unregister(1000, "camera", id));
    assert(current("/fake/cpu/scaling_min_freq") == "100");
    assert(manager.Register(1000, "bad", {"/arbitrary/path", "1"}) == 0);
    assert(manager.Register(1000, "bad", {"cpu_min"}) == 0);
    assert(manager.Acquire("ddr1", "ddr"));
    assert(manager.Acquire("ddr2", "ddr"));
    assert(current("/fake/ddr/scene_boost_dfs") == "1 800");
    auto countWrites = [&] {
        std::lock_guard lock(mutex);
        return writes.size();
    };
    auto count = countWrites();
    manager.ReleaseOwner("ddr1");
    assert(countWrites() == count);
    manager.ReleaseOwner("ddr2");
    assert(current("/fake/ddr/scene_boost_dfs") == "0 800");
    assert(manager.Acquire("ddr-retry", "ddr"));
    {
        std::lock_guard lock(mutex);
        failValue = "0 800";
    }
    manager.ReleaseOwner("ddr-retry");
    assert(current("/fake/ddr/scene_boost_dfs") == "1 800");
    {
        std::lock_guard lock(mutex);
        failValue.clear();
    }
    for (int i = 0; i < 100 && current("/fake/ddr/scene_boost_dfs") != "0 800"; ++i)
        std::this_thread::sleep_for(10ms);
    assert(current("/fake/ddr/scene_boost_dfs") == "0 800");
    auto cleanup = manager.Register(1000, "dead-client", {"cpu_min", "180"});
    assert(cleanup != 0);
    manager.Acquire("dead-client", manager.SceneName(cleanup));
    manager.ReleaseOwner("dead-client");
    manager.DropTag(1000, "dead-client");
    assert(manager.SceneId("dynamic_1") == 0);
    manager.Acquire("a", "launch");
    manager.Control(false);
    assert(current("/fake/cpu/scaling_min_freq") == "100");
    assert(!manager.Acquire("a", "launch"));
    manager.Control(true);
    assert(manager.Acquire("a", "launch"));
    manager.ReleaseOwner("a");
    assert(manager.Vip("camera", 42, 1));
    assert(current("/proc/sys/walt/sched_task_ll_camera") == "42 1");
    assert(!manager.Vip("../../other", 42, 1));
    std::cout << "PASS: scene lookup, arbitration, references, token cleanup, timed hints, dynamic "
                 "ownership, control and VIP routing\n";
}
