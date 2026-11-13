# Common hardware components for UNISOC / Spreadtrum devices

Android 13 / LineageOS 20 bring-up, initially for RongYue E5 (UMS9621).
The layout follows `hardware/mediatek`: standard AIDL HAL implementations,
init/VINTF fragments and a common product include. The standard services below
are wired into `lineage_e5`.

## Implemented

| Module | Behavior | Evidence / limits |
| --- | --- | --- |
| `android.hardware.lights-service.sprd` | Panel brightness and optional keyboard backlight; RGB luminance scaled to `max_brightness` | Stock lights binary paths; no RGB notification LED or vendor power/touch side effects |
| `android.hardware.memtrack-service.sprd` | Mali GPU-private pages by process or globally; discovers kernel GPU device name | Source kbase `gpu_memory` sysfs interface; no DMA-BUF PSS; returns unsupported when accounting is unavailable |
| `libdrm.vendor` | Explicitly installs both vendor library ABIs for stock HWC/mapper | ELF blobs copied by Make do not pull in Soong dependencies |

Memtrack reads `/sys/class/misc/mali0/device/gpu_memory` and matches the process
against **tgid**: the compatible record field labelled `tid` is `kctx->tgid`,
while the field labelled `pid` is a thread ID. Contexts use numeric indices;
the sysfs interface does not expose kernel addresses. The kernel returns an
error rather than truncating a report that exceeds one sysfs page.
All matching contexts are summed, pages are converted using the runtime page
size, and global usage is taken from the device total without counting contexts
again. CPU Mali mappings use `VM_PFNMAP` and are not accounted by smaps.
This reader is specific to the inspected UNISOC format, not every Mali driver.

The existing empty HIDL/AIDL templates are retained in separate Soong namespaces. They must **not**
replace stock libraries: method names alone do not establish transaction codes,
parcel layout, inheritance or hashes. Power/enhance, boot control, radio, TEE and
other vendor-private implementations remain stock.

The policy also supplies entrypoint/registration labels for the retained stock
power, enhance and boot services, plus yloglite. Init requires domain transitions
even in permissive mode. This is bring-up policy, not a complete enforcing policy
for every stock service or its device-node access; those still require device AVCs.
The stock `init.insmod.sh` wrapper transitions into the platform `vendor_modprobe`
domain and can set its module-ready property.

## Integration

```make
# BoardConfig.mk
BOARD_HAS_SPRD_HARDWARE := true
SPRD_SEPOLICY_PLATFORM := ums9621
include device/sprd/sepolicy_vndr/SEPolicy.mk

# device.mk
$(call inherit-product, hardware/sprd/sprd-common.mk)
```

Remove the stock lights/memtrack binaries and their init rc entries from the
blob list, then regenerate the vendor makefile. Their standard HAL declarations
now come from source `vintf_fragments`; do not also declare them in the device
manifest. E5 already applies this replacement. SELinux entrypoint labels are
included; sysfs labels on another board must match its actual device paths.
The older `hardware/sprd/sepolicy/vendor` subset can be used separately for
minimal bring-up; do not include it together with `device/sprd/sepolicy_vndr`.
`BOARD_HAS_SPRD_HARDWARE` gates Make subdirectories only; Soong module selection
is controlled by namespaces and `PRODUCT_PACKAGES`.

Lights does not force `bl_power` or enable a DRM connector: HWC/kernel still
control panel power. Memtrack uses a read-only, separately labelled sysfs
attribute. Brightness control and nonzero GPU accounting have been verified
with SELinux enforcing on Qogirn6L. HAL debugfs access remains prohibited.
Neither service claims to fix bootloader, kernel or first-stage init failures.

## Verification

From the Android source root:

```sh
source build/envsetup.sh
lunch lineage_e5-eng
m -j2 android.hardware.lights-service.sprd android.hardware.memtrack-service.sprd \
    libdrm.vendor sprd_hardware_test selinux_policy
out/host/linux-x86/nativetest64/sprd_hardware_test/sprd_hardware_test
```

Use `tools/check_blob_deps.py --root out/target/product/e5 <image-relative ELF>...`
to check DT_NEEDED recursively, including extracted or flattened VNDK/runtime APEXes.
It checks same-architecture library presence, **not** linker namespace access,
symbol/struct ABI compatibility or files loaded with dlopen. The E5 bring-up
notes contain the complete display-root command and device checks.

`tools/dump_hal_api.sh` inventories exported private HAL method signatures.
`tools/generate_hal_stubs.py --output /tmp/sprd-hal-skeleton` generates experimental
templates into a scratch directory; it refuses to overwrite this maintained tree.
