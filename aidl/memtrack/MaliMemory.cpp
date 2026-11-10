// SPDX-License-Identifier: Apache-2.0
#include "MaliMemory.h"

#include <charconv>
#include <limits>
#include <sstream>
#include <string>
#include <string_view>

namespace sprd::memtrack {
namespace {
bool parseUnsigned(std::string_view text, uint64_t* value) {
    const auto result = std::from_chars(text.data(), text.data() + text.size(), *value);
    return result.ec == std::errc{} && result.ptr == text.data() + text.size();
}
}  // namespace

std::optional<int64_t> readMaliMemory(std::istream& input, int32_t pid, int64_t pageSize) {
    if (pid < 0 || pageSize <= 0) return std::nullopt;
    uint64_t total = 0;
    const uint64_t limit = std::numeric_limits<int64_t>::max() / pageSize;
    bool sawDevice = false;
    std::string line;
    while (std::getline(input, line)) {
        if (line.find_first_not_of(" \t\r") == std::string::npos) continue;
        std::istringstream row(line);
        std::string name, count, identity, extra;
        uint64_t pages;
        if (!(row >> name >> count) || !parseUnsigned(count, &pages)) return std::nullopt;
        bool include = false;
        if (name.rfind("kctx-", 0) == 0) {
            if (!sawDevice || !(row >> identity) || identity.rfind("[pid:", 0) != 0 ||
                identity.back() != ']') return std::nullopt;
            const size_t split = identity.find(",tid:");
            if (split == std::string::npos) return std::nullopt;
            const std::string_view fields(identity);
            uint64_t tid, tgid;
            if (!parseUnsigned(fields.substr(5, split - 5), &tid) ||
                !parseUnsigned(fields.substr(split + 5, fields.size() - split - 6), &tgid)) {
                return std::nullopt;
            }
            include = pid != 0 && tgid == static_cast<uint64_t>(pid);
        } else {
            if (name.rfind("mali", 0) != 0) return std::nullopt;
            sawDevice = true;
            // Global usage includes pools not currently charged to a context.
            // Do not add the per-context rows again when pid == 0.
            include = pid == 0;
        }
        if (row >> extra) return std::nullopt;
        if (include) {
            if (pages > limit - total) return std::nullopt;
            total += pages;
        }
    }
    if (!sawDevice || input.bad()) return std::nullopt;
    return static_cast<int64_t>(total * pageSize);
}

}  // namespace sprd::memtrack
