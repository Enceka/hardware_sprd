# SPDX-License-Identifier: Apache-2.0
# Standard HAL implementations. Private interface skeletons are NOT included:
# stock clients still require their complete, ABI-compatible vendor libraries.
PRODUCT_SOONG_NAMESPACES += hardware/sprd
PRODUCT_PACKAGES += \
    android.hardware.lights-service.sprd \
    android.hardware.memtrack-service.sprd

# Prebuilt HWC/mapper libraries are copied with PRODUCT_COPY_FILES and cannot
# pull in Soong dependencies. libdrm is not supplied by the VNDK APEX.
PRODUCT_PACKAGES += libdrm.vendor
