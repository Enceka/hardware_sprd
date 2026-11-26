// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "../DdrVote.h"
int main() {
    std::vector<std::string> writes;
    std::string fail;
    bool available = false;
    sprd::power::DdrVote vote(
        "/fake/scene_boost_dfs",
        [&](const std::string& path, std::string* value) {
            assert(path == "/fake/ddrinfo_freq_table");
            if (!available) return false;
            *value = "400 0 800 255 200 800";
            return true;
        },
        [&](const std::string&, const std::string& value) {
            writes.push_back(value);
            return value != fail;
        });
    assert(vote.Resolve("max") == 0);
    available = true;
    assert(vote.Resolve("max") == 800);
    assert(vote.Resolve("min") == 200);
    assert(vote.Resolve("400") == 400);
    assert(vote.Resolve("401") == 0);
    assert(vote.Resolve("255") == 0);
    assert(vote.Resolve("-1") == 0);
    assert(vote.Resolve("400 junk") == 0);
    assert(vote.Update(800));
    assert(vote.Update(800));
    assert(writes == std::vector<std::string>{"1 800"});
    assert(vote.Update(400));
    assert((writes == std::vector<std::string>{"1 800", "0 800", "1 400"}));
    fail = "0 400";
    assert(!vote.Update(200));
    assert(vote.Current() == 400);
    fail = "1 200";
    assert(!vote.Update(200));
    assert(vote.Current() == 400);
    assert(writes.back() == "1 400");
    fail.clear();
    assert(vote.Update(0));
    auto size = writes.size();
    assert(vote.Update(0));
    assert(writes.size() == size);
    assert(vote.Current() == 0);
    std::cout << "PASS: DDR extrema, validation, balanced votes, replacement, retry and rollback\n";
}
