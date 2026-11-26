// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0
#include <android-base/file.h>
#include <android-base/test_utils.h>
#include <gtest/gtest.h>

#include "../Power.h"

using aidl::android::hardware::power::Mode;
using aidl::android::hardware::power::sprd::Power;

TEST(DoubleTapWake, NodeSupportAndWritesBypassSceneLookup) {
    TemporaryDir directory;
    const std::string path = std::string(directory.path) + "/dt2w";
    // No scene engine: dt2w must return before scene lookup on every path.
    auto power = ndk::SharedRefBase::make<Power>(nullptr, path);
    bool supported = true;
    ASSERT_TRUE(power->isModeSupported(Mode::DOUBLE_TAP_TO_WAKE, &supported).isOk());
    EXPECT_FALSE(supported);
    ASSERT_TRUE(::android::base::WriteStringToFile("0\n", path));
    ASSERT_TRUE(power->isModeSupported(Mode::DOUBLE_TAP_TO_WAKE, &supported).isOk());
    EXPECT_TRUE(supported);
    ASSERT_TRUE(power->setMode(Mode::DOUBLE_TAP_TO_WAKE, true).isOk());
    std::string value;
    ASSERT_TRUE(::android::base::ReadFileToString(path, &value));
    EXPECT_EQ("1\n", value);
    ASSERT_TRUE(power->setMode(Mode::DOUBLE_TAP_TO_WAKE, false).isOk());
    ASSERT_TRUE(::android::base::ReadFileToString(path, &value));
    EXPECT_EQ("0\n", value);
}

TEST(DoubleTapWake, MissingConfigurationIsUnsupported) {
    auto power = ndk::SharedRefBase::make<Power>(nullptr, "");
    bool supported = true;
    ASSERT_TRUE(power->isModeSupported(Mode::DOUBLE_TAP_TO_WAKE, &supported).isOk());
    EXPECT_FALSE(supported);
    EXPECT_EQ(EX_UNSUPPORTED_OPERATION,
              power->setMode(Mode::DOUBLE_TAP_TO_WAKE, true).getExceptionCode());
}

TEST(DoubleTapWake, WriteFailureReturnsIllegalState) {
    TemporaryDir directory;
    auto power = ndk::SharedRefBase::make<Power>(nullptr, directory.path);
    EXPECT_EQ(EX_ILLEGAL_STATE, power->setMode(Mode::DOUBLE_TAP_TO_WAKE, true).getExceptionCode());
}

TEST(Interactive, NodeWritesDoNotRequireSceneActions) {
    TemporaryDir directory;
    const std::string path = std::string(directory.path) + "/interactive";
    ASSERT_TRUE(::android::base::WriteStringToFile("1\n", path));
    auto scenes = std::make_shared<::sprd::power::SceneManager>(
        [](const std::string&, std::string*) { return false; },
        [](const std::string&, const std::string&) { return false; });
    auto power = ndk::SharedRefBase::make<Power>(scenes, "", path);
    ASSERT_TRUE(power->setMode(Mode::INTERACTIVE, false).isOk());
    std::string value;
    ASSERT_TRUE(::android::base::ReadFileToString(path, &value));
    EXPECT_EQ("0\n", value);
    ASSERT_TRUE(power->setMode(Mode::INTERACTIVE, true).isOk());
    ASSERT_TRUE(::android::base::ReadFileToString(path, &value));
    EXPECT_EQ("1\n", value);
}

TEST(Interactive, WriteFailureReturnsIllegalState) {
    TemporaryDir directory;
    auto power = ndk::SharedRefBase::make<Power>(nullptr, "", directory.path);
    EXPECT_EQ(EX_ILLEGAL_STATE, power->setMode(Mode::INTERACTIVE, false).getExceptionCode());
}
