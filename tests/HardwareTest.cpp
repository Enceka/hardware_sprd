// SPDX-License-Identifier: Apache-2.0
#include <gtest/gtest.h>

#include <sstream>

#include "../aidl/light/Backlight.h"
#include "../aidl/memtrack/MaliMemory.h"

namespace {
std::optional<int64_t> memory(const std::string& text, int pid, int pageSize = 4096) {
    std::istringstream input(text);
    return sprd::memtrack::readMaliMemory(input, pid, pageSize);
}

constexpr char kMemory[] =
        "mali0                 25\n"
        "  kctx-0x1234         10 [pid:101,tid:100]\n"
        "  kctx-0x2345          4 [pid:102,tid:100]\n"
        "  kctx-0x3456          3 [pid:200,tid:200]\n";

TEST(MaliMemory, SumsContextsByThreadGroupDespiteMisleadingLabels) {
    EXPECT_EQ(memory(kMemory, 100), 14 * 4096);
    EXPECT_EQ(memory(kMemory, 101), 0);
    EXPECT_EQ(memory(kMemory, 200), 3 * 4096);
}

TEST(MaliMemory, GlobalIncludesPoolWithoutDoubleCountingContexts) {
    EXPECT_EQ(memory(kMemory, 0), 25 * 4096);
    EXPECT_EQ(memory(kMemory, 0, 16384), 25 * 16384);
}

TEST(MaliMemory, RejectsUnreadableMalformedAndOverflowingInput) {
    EXPECT_FALSE(memory("", 0));
    EXPECT_FALSE(memory("mali0 -1\n", 0));
    EXPECT_FALSE(memory("mali0 1\ntruncated\n", 0));
    EXPECT_FALSE(memory("mali0 18446744073709551616\n", 0));
    EXPECT_FALSE(memory("mali0 9223372036854775807\n", 0));
    EXPECT_FALSE(memory("mali0 1\n kctx-0x1 1 [pid:1,tid:x]\n", 1));
    EXPECT_FALSE(memory(kMemory, -1));
    EXPECT_FALSE(memory(kMemory, 0, 0));
}

TEST(Backlight, ScalesRgbLuminanceAndIgnoresAlpha) {
    EXPECT_EQ(sprd::light::brightness(0xffffffff, 4095), 4095);
    EXPECT_EQ(sprd::light::brightness(0x00ffffff, 4095), 4095);
    EXPECT_EQ(sprd::light::brightness(0xff000000, 255), 0);
    EXPECT_EQ(sprd::light::brightness(0xffff0000, 255), 76);
    EXPECT_EQ(sprd::light::brightness(0xff00ff00, 255), 149);
    EXPECT_EQ(sprd::light::brightness(0xff0000ff, 255), 28);
    EXPECT_EQ(sprd::light::brightness(0xff808080, 255), 128);
    EXPECT_EQ(sprd::light::brightness(0xffffffff, 2147483647), 2147483647);
}
}  // namespace
