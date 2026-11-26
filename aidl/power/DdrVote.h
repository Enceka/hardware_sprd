// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace sprd::power {
// One kernel "boost" reference for the aggregate of all HAL requests.
// The caller serializes access. Frequencies retain the firmware table units.
class DdrVote {
   public:
    using Read = std::function<bool(const std::string&, std::string*)>;
    using Write = std::function<bool(const std::string&, const std::string&)>;
    DdrVote(std::string path, Read read, Write write);
    uint32_t Resolve(const std::string& value);
    bool Update(uint32_t frequency);
    uint32_t Current() const { return mCurrent; }

   private:
    std::string mPath;
    Read mRead;
    Write mWrite;
    std::vector<uint32_t> mFrequencies;
    uint32_t mCurrent = 0;
};
}  // namespace sprd::power
