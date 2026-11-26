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
    std::map<std::string, std::string> files{{"/fake/cpu/min", "100"},
                                             {"/fake/cpu/max", "300"},
                                             {"/fake/ddr/ddrinfo_freq_table", "200 800 400 255 0"}};
    std::vector<std::pair<std::string, std::string>> writes;
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
    assert(manager.Acquire("a", "launch"));
    assert(manager.Acquire("b", "interaction_fling"));
    assert(current("/fake/cpu/min") == "200");
    manager.ReleaseOwner("a");
    assert(current("/fake/cpu/min") == "150");
    manager.ReleaseOwner("b");
    assert(current("/fake/cpu/min") == "100");
    assert(manager.Acquire("a", "launch"));
    assert(manager.Acquire("a", "launch"));
    manager.Release("a", "launch");
    assert(current("/fake/cpu/min") == "200");
    manager.Release("a", "launch");
    assert(current("/fake/cpu/min") == "100");
    manager.DoHint("INTERACTION", 20ms);
    manager.Acquire("private", "launch");
    std::this_thread::sleep_for(50ms);
    assert(current("/fake/cpu/min") == "200");
    manager.ReleaseOwner("private");
    assert(current("/fake/cpu/min") == "100");
    int id = manager.Register(1000, "camera", {"cpu_min", "180"});
    assert(id != 0);
    assert(manager.Acquire("camera", manager.SceneName(id)));
    assert(current("/fake/cpu/min") == "180");
    assert(!manager.Unregister(1001, "camera", id));
    assert(!manager.Unregister(1000, "other", id));
    assert(manager.Unregister(1000, "camera", id));
    assert(current("/fake/cpu/min") == "100");
    assert(manager.Register(1000, "bad", {"/arbitrary/path", "1"}) == 0);
    assert(manager.Register(1000, "bad", {"cpu_min"}) == 0);
    manager.Acquire("a", "launch");
    manager.Control(false);
    assert(current("/fake/cpu/min") == "100");
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
