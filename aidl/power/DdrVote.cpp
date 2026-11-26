// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

#include "DdrVote.h"

#include <algorithm>
#include <limits>
#include <sstream>

namespace sprd::power {
DdrVote::DdrVote(std::string path, Read read, Write write)
    : mPath(std::move(path)), mRead(std::move(read)), mWrite(std::move(write)) {}
uint32_t DdrVote::Resolve(const std::string& value) {
    if (mFrequencies.empty()) {
        std::string table;
        auto slash = mPath.rfind('/');
        if (!mRead(mPath.substr(0, slash + 1) + "ddrinfo_freq_table", &table)) return 0;
        std::istringstream values(table);
        uint64_t frequency;
        while (values >> frequency) {
            if (frequency > 0 && frequency != 0xff &&
                frequency <= std::numeric_limits<uint32_t>::max())
                mFrequencies.push_back(frequency);
        }
        std::sort(mFrequencies.begin(), mFrequencies.end());
        mFrequencies.erase(std::unique(mFrequencies.begin(), mFrequencies.end()),
                           mFrequencies.end());
    }
    if (mFrequencies.empty()) return 0;
    if (value == "max") return mFrequencies.back();
    if (value == "min") return mFrequencies.front();
    std::istringstream number(value);
    uint64_t frequency;
    std::string trailing;
    if (!(number >> frequency) || number >> trailing ||
        frequency > std::numeric_limits<uint32_t>::max())
        return 0;
    return std::binary_search(mFrequencies.begin(), mFrequencies.end(), frequency) ? frequency : 0;
}
bool DdrVote::Update(uint32_t frequency) {
    if (frequency == mCurrent) return true;
    auto previous = mCurrent;
    if (previous) {
        if (!mWrite(mPath, "0 " + std::to_string(previous))) return false;
        mCurrent = 0;
    }
    if (!frequency) return true;
    if (mWrite(mPath, "1 " + std::to_string(frequency))) {
        mCurrent = frequency;
        return true;
    }
    // A failed replacement must not silently discard the previous vote.
    if (previous && mWrite(mPath, "1 " + std::to_string(previous))) mCurrent = previous;
    return false;
}
}  // namespace sprd::power
