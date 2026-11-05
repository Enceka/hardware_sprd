# SPRD hardware

UNISOC / Spreadtrum hardware components for Android 13 / LineageOS 20.

- Lights: panel brightness and optional keyboard backlight.
- Memtrack: Mali GPU memory accounting through the kernel's `gpu_memory` sysfs node.
- USB: AIDL port status and role switching through SPRD kernel nodes.
- Private HIDL/AIDL interfaces and tools for inspecting stock blob dependencies.

## Integration

```make
# BoardConfig.mk
BOARD_HAS_SPRD_HARDWARE := true
SPRD_SEPOLICY_PLATFORM := ums9621
include device/sprd/sepolicy_vndr/SEPolicy.mk

# device.mk
$(call inherit-product, hardware/sprd/sprd-common.mk)
```

The common include selects lights and Memtrack. Select
`android.hardware.usb-service.sprd` separately for supported boards. Remove
replaced stock services and duplicate VINTF entries from the device tree.
Adapt SELinux labels to the board's actual sysfs paths.

Private interface stubs are experimental and excluded from the common product;
retain the stock implementations required by proprietary clients.

## Checks

After selecting the product with `lunch`:

```sh
m sprd_hardware_test
out/host/linux-x86/nativetest64/sprd_hardware_test/sprd_hardware_test
```

`tools/check_blob_deps.py` checks ELF library dependencies against a product
output tree. `tools/dump_hal_api.sh` inspects private HAL interfaces.
