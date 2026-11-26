# SPRD AIDL Power HAL

Source implementation of Android Power AIDL v3, backed by the existing open
source libperfmgr. There is no dependency on power.sprd.so or libpowerhal_cli.
Install android.hardware.power-service.sprd and /vendor/etc/powerhint.json.
An alternative config path may be supplied as the first service argument.

The JSON uses libperfmgr Nodes/Actions. Enum names map directly to hints;
only configured hints are advertised. A zero boost duration uses the configured
timeout, positive durations override it and negative durations cancel it.
Mode actions use Duration 0 until disabled. INTERACTIVE gates transient boosts;
screen off and LOW_POWER cancel all boosts and LAUNCH. If configured,
DISPLAY_INACTIVE is enabled on screen off and released on screen on.
ADPF sessions are reported as unsupported. dumpsys android.hardware.power.IPower/default
shows libperfmgr request statistics.

## E5 integration

Build with E5_USE_SOURCE_POWER_HAL=true to replace the stock executable and
power.sprd module. The normal build retains the stock service pending private
client migration: the OEM executable also owns vendor.unisoc.hardware.power
IVendorPower. Its scene leases, registration and VIP task APIs are used by
vendor clients and are not implemented by this standard service. Do not install
both standard service providers together.

E5 powerhint.json takes CPU boost frequencies from the stock
power_scene_config.xml: policy0 2184000, policy6 2301000, policy7 2704000 kHz.
Only minimum frequencies are raised; release writes 0 so cpufreq returns to
its hardware minimum. The scheduler and thermal frequency ceilings remain active.
Interaction lasts 150 ms by default, camera launch 1000 ms, and LAUNCH is held
until disabled. These timeout values are initial tuning choices.

DDR max is intentionally omitted: the stock library converts the symbolic
scene to the kernel's enable/frequency pair and balances its vote reference
count. A persistent libperfmgr node needs that protocol handled explicitly.
Low-power, sustained/fixed-performance and camera-streaming policies require
separate device tuning before adding their hint actions.

## Validation

Build: m android.hardware.power-service.sprd
On a test device, enable the source flag and verify a single IPower/default,
launch/interaction frequency increases, timeout restoration, overlapping
launch/interaction release, screen-off cancellation, thermal limiting, and
camera/video operation. Check logcat and SELinux denials. Private IVendorPower
client migration is required before enabling the source service by default.

Reference architecture: https://github.com/XagaForge/android_hardware_mediatek
(branch 16.2, hidl/mtkpower/1.2/default/MtkPower.cpp), which routes vendor hints
to a standard AIDL Power service. This implementation uses the local AOSP v3
interface and libperfmgr rather than MediaTek-specific interfaces.
