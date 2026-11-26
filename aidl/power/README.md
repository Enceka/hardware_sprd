# SPRD AIDL Power HAL

The source service provides Android Power AIDL v3 and the nine-method Unisoc
IVendorPower V1 interface. Both use a shared XML scene manager so private camera
leases and standard framework hints arbitrate the same nodes. No power.sprd.so
or libpowerhal_cli is loaded by the service. Existing proprietary client libraries
remain unchanged.

Install android.hardware.power-service.sprd and the OEM power_scene_config.xml,
power_scene_id_define.txt, power_resource_file_info.xml. The first executable
argument can override the default configuration directory /vendor/etc.
E5_USE_SOURCE_POWER_HAL=true selects this service on E5 and removes the OEM
service and its init rc. Keep the opt-in until on-device camera, performance,
suspend and SELinux validation is complete.

## Private wire interface

The recovered AIDL in compat documents transaction IDs 1 through 9 in declaration
order. Transactions 1-4, 8 and 9 are oneway. Metadata reports version 1 and OEM hash
67a6288e6ac1c48aa2d0f1c882d182d74ad6f61b. The custom NDK dispatcher preserves
that metadata and uses standard NDK parcel encoding. The separately named build
interface prevents shadowing the stock library used by vendor clients.
Evidence: E5 vendor.unisoc.hardware.power-V1-ndk.so exported Bp signatures and
AIBinder_transact call sites; power.sprd.so power_impl dispatch, reg_scene,
ctrl_power_hint, set_viptask; kernel drivers/unisoc_platform/sched/sysctl.c.

Token/tag leases are reference counted. Binder death releases all leases for the
token. Dynamic scenes take resource-name/value pairs, use the OEM five dynamic
IDs, validate resource names against XML and enforce UID/tag ownership on
unregister. Death also removes dynamic registrations for a tag after its last
linked token dies. Disabling hints releases all outstanding requests. VIP types audio,
ui, camera and animator write pid/level pairs to their WALT nodes.

Resource defaults are captured before first use (or taken from explicit XML
def_value attributes) and restored after the last request. Numeric resources
and vectors use their configured ascending/descending arbitration. Standard
modes are persistent until disabled; interaction defaults to 150 ms and camera
launch/shot to 1000 ms. These timeouts are initial tuning choices. Screen off and
LOW_POWER cancel transient standard hints. Empty XML policies are not advertised.
ADPF sessions remain unsupported. dumpsys android.hardware.power.IPower/default
shows active leases and applied/baseline node values.

The engine supports common_set resources and expands subsystem configurations
into their constituent controls. Other resource backends are rejected when a
scene references them; missing/unreadable nodes are skipped without inventing
baseline values. The standard adapter maps enum names to lowercase OEM scene
names, with interaction_fling, camera_high_perf and screen_rotation aliases.
Only the normal OEM policy group is loaded; E5's other groups are empty.

## Host checks

From the Android source root:

```sh
g++ -std=c++17 -Wall -Wextra -Werror -pthread -Iexternal/tinyxml2 \
  hardware/sprd/aidl/power/tests/SceneManagerTest.cpp \
  hardware/sprd/aidl/power/SceneManager.cpp external/tinyxml2/tinyxml2.cpp \
  -o /tmp/sprd-scene-test
/tmp/sprd-scene-test hardware/sprd/aidl/power/tests/data
/tmp/sprd-scene-test vendor/rongyue/e5/proprietary/vendor/etc --load
```

Build with m android.hardware.power-service.sprd. On device, verify both services
register, stock camera clients can acquire/release scenes, killing a client
releases its leases, standard/private overlap restores correctly, timeout and
screen-off cancellation work, and there are no SELinux denials.
