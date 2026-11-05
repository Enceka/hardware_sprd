#!/bin/sh
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#
# Generate blueprint/makefiles for the vendor HALs.
set -e
cd "$(dirname "$0")/.."
exec python3 tools/generate_hal_stubs.py
