/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

# Common hardware components for UNISOC / Spreadtrum devices

Source-side definitions of the vendor-private HAL interfaces found on the RongYue E5
(`ums9158_1h10` / `ums9621`, Android 13 / LineageOS 20), plus no-op stub services that
can replace their stock counterparts one by one.

## Status

This repository is **not wired into any product yet**. It is deliberately independent
of `device/rongyue/e5` and of the repo manifest. Nothing here changes the current
boot behaviour of the device.

The interface declarations use the exact package / interface / instance names from
`vendor/product/etc/vintf/compatibility_matrix.xml` and `vendor/etc/vintf/manifest*.xml`
of the stock image, but their **method bodies are empty**. The stock prebuilt interface
libraries do not ship `.hal` / `.aidl` sources, and C++ mangling does not encode return
types, so the real transaction signatures cannot be reconstructed losslessly. Before
switching a real client over to a stub, fill in its methods using
`tools/dump_hal_api.sh` and verify the transaction codes.

## Layout

| Path | Purpose |
| --- | --- |
| `interfaces/` | `hidl_package_root` declarations for the two vendor roots |
| `interfaces/hidl/` | HIDL `.hal` definitions, one directory per package/version |
| `interfaces/hidl_sw/` | `vendor.sw.swfingerprint` HIDL definitions |
| `interfaces/aidl/` | `aidl_interface` modules, `.aidl` sources and `aidl_api/current` dumps |
| `hidl/` | binderized HIDL stub services, init rc and VINTF fragments |
| `aidl/` | AIDL stub services, init rc and VINTF fragments |
| `tools/` | inventory generator and blob API dumper |

The build is gated by `BOARD_HAS_SPRD_HARDWARE`, mirroring
`hardware/mediatek`'s `BOARD_HAS_MTK_HARDWARE`.

## HIDL inventory

| Package | Interfaces (instances) | Stock interface library |
| --- | --- | --- |
| `vendor.sprd.hardware.boot@1.2` | IBootControl (default) | `vendor.sprd.hardware.boot@1.2.so` |
| `vendor.sprd.hardware.connmgr@1.0` | IConnmgr (default), IConnmgrCallback (callback) | `vendor.sprd.hardware.connmgr@1.0.so` |
| `vendor.sprd.hardware.cplog_svc@1.0` | ILogTransportInductor (default), ISubsysLogTransport (callback) | `vendor.sprd.hardware.cplog_svc@1.0.so` |
| `vendor.sprd.hardware.gnss@1.0` | IUnisocGnss (default), IUnisocAGnssRil (callback), IUnisocGnssConfiguration (callback), IUnisocGnssPowerIndication (callback), IUnisocGnssPowerIndicationCallback (callback) | `vendor.sprd.hardware.gnss@1.0.so` |
| `vendor.sprd.hardware.hdcp@1.0` | IHDCP (default) | `vendor.sprd.hardware.hdcp@1.0.so` |
| `vendor.sprd.hardware.face@1.0` | IExtBiometricsFace (default) | n/a |
| `vendor.sprd.hardware.ifaa@1.0` | IIfaa (default) | n/a |
| `vendor.sprd.hardware.log@1.0` | ILogControl (default) | `vendor.sprd.hardware.log@1.0.so` |
| `vendor.sprd.hardware.network@1.0` | INetworkControl (default) | `vendor.sprd.hardware.network@1.0.so` |
| `vendor.sprd.hardware.power@4.0` | IPower (default) | n/a |
| `vendor.sprd.hardware.radio@1.1` | IExtRadio (slot1/slot2) | n/a |
| `vendor.sprd.hardware.radio.ims@1.1` | IImsRadio (slot1/slot2) | n/a |
| `vendor.sprd.hardware.radio.lite@1.0` | ILiteRadio (liteservice1/liteservice2) | n/a |
| `vendor.sprd.hardware.thermal@1.0` | IExtThermal (default) | `vendor.sprd.hardware.thermal@1.0.so` |
| `vendor.sprd.hardware.thermal@2.0` | IExtThermal (default) | `vendor.sprd.hardware.thermal@2.0.so` |
| `vendor.sprd.hardware.trusty@1.0` | ITrustyClientProvider (default) | `vendor.sprd.hardware.trusty@1.0.so` |
| `vendor.sprd.hardware.tui@1.0` | ITui (default) | `vendor.sprd.hardware.tui@1.0.so` |
| `vendor.sprd.hardware.vdsp@1.0` | IVdspService (default) | n/a |
| `vendor.sprd.hardware.wcn@1.0` | IWcnVendorInterface (default) | `vendor.sprd.hardware.wcn@1.0.so` |
| `vendor.sprd.hardware.wifi.hostapd@1.1` | IHostapd (default) | n/a |
| `vendor.sw.swfingerprint@1.0` | ISwfingerprint (default) | n/a |

## AIDL inventory

| Package | Interfaces (instances) | Stock interface library |
| --- | --- | --- |
| `vendor.sprd.hardware.aprd` | IAprdInfoSync (default) | n/a |
| `vendor.sprd.hardware.cplog_svc` | ISysLogControl (default), ILogTransportInductor (default) | `vendor.sprd.hardware.cplog_svc-V1-ndk.so` |
| `vendor.sprd.hardware.enhance` | IEnhance (default) | `vendor.sprd.hardware.enhance-V1-ndk.so` |
| `vendor.sprd.hardware.fingerprintmmi` | IFingerprintmmi (default) | `vendor.sprd.hardware.fingerprintmmi-V1-ndk.so` |
| `vendor.sprd.hardware.hdcp` | IHDCP (default) | n/a |
| `vendor.sprd.hardware.ifaa` | IIfaa (default) | n/a |
| `vendor.sprd.hardware.log` | ILogControl (default) | `vendor.sprd.hardware.log-V1-ndk.so` |
| `vendor.sprd.hardware.network` | INetworkControl (default) | `vendor.sprd.hardware.network-V1-ndk.so` |
| `vendor.sprd.hardware.soter` | ISoter (default) | `vendor.sprd.hardware.soter-V1-ndk.so` |
| `vendor.sprd.hardware.trusty` | ITrustyClientProvider (default) | `vendor.sprd.hardware.trusty-V1-ndk.so` |
| `vendor.sprd.hardware.tui` | ITui (default) | `vendor.sprd.hardware.tui-V1-ndk.so` |
| `vendor.unisoc.hardware.ai_engine` | IAIEngineControl (default) | n/a |
| `vendor.unisoc.hardware.power` | IVendorPower (default) | `vendor.unisoc.hardware.power-V1-ndk.so` |
| `vendor.unisoc.hardware.radio` | IExtRadio (slot1/slot2) | `vendor.unisoc.hardware.radio-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.data` | IExtRadioData (slot1/slot2) | `vendor.unisoc.hardware.radio.data-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.ims` | IImsRadio (slot1/slot2) | `vendor.unisoc.hardware.radio.ims-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.lite` | ILiteRadio (liteservice1/liteservice2) | `vendor.unisoc.hardware.radio.lite-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.messaging` | IExtRadioMessaging (slot1/slot2) | `vendor.unisoc.hardware.radio.messaging-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.modem` | IExtRadioModem (slot1/slot2) | `vendor.unisoc.hardware.radio.modem-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.network` | IExtRadioNetwork (slot1/slot2) | `vendor.unisoc.hardware.radio.network-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.sim` | IExtRadioSim (slot1/slot2) | `vendor.unisoc.hardware.radio.sim-V1-ndk.so` |
| `vendor.unisoc.hardware.radio.voice` | IExtRadioVoice (slot1/slot2) | `vendor.unisoc.hardware.radio.voice-V1-ndk.so` |

## Stub services

| Module | Registers |
| --- | --- |
| `vendor.sprd.hardware.boot@1.2-service.stub` | vendor.sprd.hardware.boot@1.2 |
| `vendor.sprd.hardware-service.stub` | vendor.sprd.hardware.connmgr@1.0, vendor.sprd.hardware.cplog_svc@1.0, vendor.sprd.hardware.gnss@1.0, vendor.sprd.hardware.hdcp@1.0, vendor.sprd.hardware.face@1.0, vendor.sprd.hardware.ifaa@1.0, vendor.sprd.hardware.log@1.0, vendor.sprd.hardware.network@1.0, vendor.sprd.hardware.power@4.0, vendor.sprd.hardware.radio@1.1, vendor.sprd.hardware.radio.ims@1.1, vendor.sprd.hardware.radio.lite@1.0, vendor.sprd.hardware.thermal@1.0, vendor.sprd.hardware.thermal@2.0, vendor.sprd.hardware.trusty@1.0, vendor.sprd.hardware.tui@1.0, vendor.sprd.hardware.vdsp@1.0, vendor.sprd.hardware.wcn@1.0, vendor.sprd.hardware.wifi.hostapd@1.1 |
| `vendor.sw.swfingerprint@1.0-service.stub` | vendor.sw.swfingerprint@1.0 |
| `vendor.sprd.hardware-aidl-service.stub` | vendor.sprd.hardware.aprd, vendor.sprd.hardware.cplog_svc, vendor.sprd.hardware.enhance, vendor.sprd.hardware.fingerprintmmi, vendor.sprd.hardware.hdcp, vendor.sprd.hardware.ifaa, vendor.sprd.hardware.log, vendor.sprd.hardware.network, vendor.sprd.hardware.soter, vendor.sprd.hardware.trusty, vendor.sprd.hardware.tui |
| `vendor.unisoc.hardware-aidl-service.stub` | vendor.unisoc.hardware.ai_engine, vendor.unisoc.hardware.power |
| `vendor.unisoc.hardware.radio-service.stub` | vendor.unisoc.hardware.radio, vendor.unisoc.hardware.radio.data, vendor.unisoc.hardware.radio.ims, vendor.unisoc.hardware.radio.lite, vendor.unisoc.hardware.radio.messaging, vendor.unisoc.hardware.radio.modem, vendor.unisoc.hardware.radio.network, vendor.unisoc.hardware.radio.sim, vendor.unisoc.hardware.radio.voice |

## Integration (not applied)

```make
# BoardConfig.mk
BOARD_HAS_SPRD_HARDWARE := true

# device.mk
PRODUCT_SOONG_NAMESPACES += hardware/sprd
PRODUCT_PACKAGES += \
    vendor.sprd.hardware.boot@1.2-service.stub \
    vendor.sprd.hardware-service.stub \
    vendor.sprd.hardware-aidl-service.stub \
    vendor.unisoc.hardware-aidl-service.stub \
    vendor.unisoc.hardware.radio-service.stub
```

## Conflicts with the stock blobs

The stock image already installs interface libraries with the same names
(`vendor.sprd.hardware.*@1.x.so`, `*-impl.so`, `*-V1-ndk.so`) and runs matching
services from `vendor/bin/hw`. Enabling a stub service therefore requires, for each
HAL:

1. removing or disabling the corresponding `vendor/etc/init/*.rc` entry and
   `vendor/bin/hw/*-service` from `proprietary-files.txt`;
2. keeping the stock interface library out of the vendor partition, or making the
   source-built one win the linker namespace;
3. deleting the hand-written declaration from
   `device/rongyue/e5/vintf/manifest.xml` so it comes from the stub's
   `vintf_fragments` instead;
4. removing the matching entry from `framework_compatibility_matrix.xml` if the
   HAL is no longer optional.

The AIDL interfaces are unfrozen (`owner` set, no `versions`), so the generated
NDK library is `<name>-V1-ndk`. Freeze them with `m <name>-freeze-api` once the
method set is final.

## Regenerating

```sh
python3 tools/generate_hal_stubs.py
BLOBS=/home/hema/Workspace/e5/vendor tools/dump_hal_api.sh
```
