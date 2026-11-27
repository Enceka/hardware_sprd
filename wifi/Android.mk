# SPDX-FileCopyrightText: 2026 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := lib_driver_cmd_sprd
LOCAL_VENDOR_MODULE := true
LOCAL_LICENSE_KINDS := SPDX-license-identifier-Apache-2.0
LOCAL_LICENSE_CONDITIONS := notice
LOCAL_SRC_FILES := driver_cmd_nl80211.c
LOCAL_C_INCLUDES := external/wpa_supplicant_8/src \
    external/wpa_supplicant_8/src/utils \
    external/wpa_supplicant_8/src/drivers
LOCAL_CFLAGS := -DANDROID -DANDROID_P2P -Wall -Wextra -Werror -Wno-unused-parameter
# The supplicant calls this library through CFI-checked driver callbacks.
LOCAL_SANITIZE := cfi
include $(BUILD_STATIC_LIBRARY)
