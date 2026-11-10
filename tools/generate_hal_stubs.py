#!/usr/bin/env python3
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#
"""Regenerate the Spreadtrum private HAL interface and stub-service skeleton.

The inventory below is derived from the RongYue E5 stock image:
  * /home/hema/Workspace/e5/vendor/product/etc/vintf/compatibility_matrix.xml
  * /home/hema/Workspace/e5/vendor/vendor/etc/vintf/manifest*.xml
  * the vendor.sprd.hardware.* / vendor.unisoc.hardware.* libraries in the
    vendor and system_ext partitions.

Run from anywhere with a new scratch directory:
  python3 tools/generate_hal_stubs.py --output /tmp/sprd-hal-skeleton
"""

import argparse
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

LICENSE_C = """/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */
"""

LICENSE_HAL = """/*
 * SPDX-FileCopyrightText: The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */
"""

# ---------------------------------------------------------------------------
# HIDL inventory. "dir" is relative to interfaces/hidl (or interfaces/hidl_sw
# for the vendor.sw root). "ifaces" maps an interface name to the instance
# names its stub service should register; an empty list means a callback
# interface that is declared but never registered as a service.
# ---------------------------------------------------------------------------
HIDL_ROOTS = [
    ("vendor.sprd.hardware", "hardware/sprd/interfaces/hidl", "hidl"),
    ("vendor.sw.swfingerprint", "hardware/sprd/interfaces/hidl_sw", "hidl_sw"),
]

HIDL = [
    dict(root="vendor.sprd.hardware", tree="hidl", dir="boot/1.2", pkg="vendor.sprd.hardware.boot", ver="1.2",
         ifaces={"IBootControl": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.boot@1.2.so",
         methods=["getNumberSlots", "getCurrentSlot", "getActiveBootSlot", "markBootSuccessful",
                  "setActiveBootSlot", "setSlotAsUnbootable", "isSlotBootable", "isSlotMarkedSuccessful",
                  "getSuffix", "setSnapshotMergeStatus", "getSnapshotMergeStatus",
                  "SetSpldestroyBeforeSetactiveFlag"]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="connmgr/1.0", pkg="vendor.sprd.hardware.connmgr", ver="1.0",
         ifaces={"IConnmgr": ["default"], "IConnmgrCallback": []}, ext=None,
         lib="vendor.sprd.hardware.connmgr@1.0.so",
         methods=["registerCallback", "SendStringCommand"]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="cplog_svc/1.0", pkg="vendor.sprd.hardware.cplog_svc", ver="1.0",
         ifaces={"ILogTransportInductor": ["default"], "ISubsysLogTransport": []}, ext=None,
         lib="vendor.sprd.hardware.cplog_svc@1.0.so",
         methods=["notifyDumpState", "setTransportInterface", "collectLog", "clear", "flush",
                  "enableLogMode", "disableLogMode", "getLogDestination"]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="gnss/1.0", pkg="vendor.sprd.hardware.gnss", ver="1.0",
         ifaces={"IUnisocGnss": ["default"], "IUnisocAGnssRil": [], "IUnisocGnssConfiguration": [],
                 "IUnisocGnssPowerIndication": [], "IUnisocGnssPowerIndicationCallback": []}, ext=None,
         lib="vendor.sprd.hardware.gnss@1.0.so",
         methods=["getExtensionAGnssRil", "getExtensionGnssConfiguration", "getExtensionGnssPowerIndication",
                  "setRefLocation", "agpsNiMessage", "setGnssLogSwitch", "setSuplCertificate"]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="hdcp/1.0", pkg="vendor.sprd.hardware.hdcp", ver="1.0",
         ifaces={"IHDCP": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.hdcp@1.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="face/1.0", pkg="vendor.sprd.hardware.face", ver="1.0",
         ifaces={"IExtBiometricsFace": ["default"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="ifaa/1.0", pkg="vendor.sprd.hardware.ifaa", ver="1.0",
         ifaces={"IIfaa": ["default"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="log/1.0", pkg="vendor.sprd.hardware.log", ver="1.0",
         ifaces={"ILogControl": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.log@1.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="network/1.0", pkg="vendor.sprd.hardware.network", ver="1.0",
         ifaces={"INetworkControl": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.network@1.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="power/4.0", pkg="vendor.sprd.hardware.power", ver="4.0",
         ifaces={"IPower": ["default"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="radio/1.1", pkg="vendor.sprd.hardware.radio", ver="1.1",
         ifaces={"IExtRadio": ["slot1", "slot2"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="radio/ims/1.1", pkg="vendor.sprd.hardware.radio.ims", ver="1.1",
         ifaces={"IImsRadio": ["slot1", "slot2"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="radio/lite/1.0", pkg="vendor.sprd.hardware.radio.lite", ver="1.0",
         ifaces={"ILiteRadio": ["liteservice1", "liteservice2"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="thermal/1.0", pkg="vendor.sprd.hardware.thermal", ver="1.0",
         ifaces={"IExtThermal": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.thermal@1.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="thermal/2.0", pkg="vendor.sprd.hardware.thermal", ver="2.0",
         ifaces={"IExtThermal": ["default"]}, ext="@1.0::IExtThermal",
         ext_lib="vendor.sprd.hardware.thermal@1.0",
         lib="vendor.sprd.hardware.thermal@2.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="trusty/1.0", pkg="vendor.sprd.hardware.trusty", ver="1.0",
         ifaces={"ITrustyClientProvider": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.trusty@1.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="tui/1.0", pkg="vendor.sprd.hardware.tui", ver="1.0",
         ifaces={"ITui": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.tui@1.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="vdsp/1.0", pkg="vendor.sprd.hardware.vdsp", ver="1.0",
         ifaces={"IVdspService": ["default"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="wcn/1.0", pkg="vendor.sprd.hardware.wcn", ver="1.0",
         ifaces={"IWcnVendorInterface": ["default"]}, ext=None,
         lib="vendor.sprd.hardware.wcn@1.0.so", methods=[]),
    dict(root="vendor.sprd.hardware", tree="hidl", dir="wifi/hostapd/1.1", pkg="vendor.sprd.hardware.wifi.hostapd", ver="1.1",
         ifaces={"IHostapd": ["default"]}, ext=None,
         lib=None, methods=[]),
    dict(root="vendor.sw.swfingerprint", tree="hidl_sw", dir="1.0", pkg="vendor.sw.swfingerprint", ver="1.0",
         ifaces={"ISwfingerprint": ["default"]}, ext=None,
         lib=None, methods=[]),
]

# ---------------------------------------------------------------------------
# AIDL inventory. "ifaces" maps an interface name to its instances.
# ---------------------------------------------------------------------------
AIDL = [
    dict(pkg="vendor.sprd.hardware.aprd", ifaces={"IAprdInfoSync": ["default"]}, lib=None),
    dict(pkg="vendor.sprd.hardware.cplog_svc",
         ifaces={"ISysLogControl": ["default"], "ILogTransportInductor": ["default"]},
         lib="vendor.sprd.hardware.cplog_svc-V1-ndk.so"),
    dict(pkg="vendor.sprd.hardware.enhance", ifaces={"IEnhance": ["default"]},
         lib="vendor.sprd.hardware.enhance-V1-ndk.so"),
    dict(pkg="vendor.sprd.hardware.fingerprintmmi", ifaces={"IFingerprintmmi": ["default"]},
         lib="vendor.sprd.hardware.fingerprintmmi-V1-ndk.so"),
    dict(pkg="vendor.sprd.hardware.hdcp", ifaces={"IHDCP": ["default"]}, lib=None),
    dict(pkg="vendor.sprd.hardware.ifaa", ifaces={"IIfaa": ["default"]}, lib=None),
    dict(pkg="vendor.sprd.hardware.log", ifaces={"ILogControl": ["default"]},
         lib="vendor.sprd.hardware.log-V1-ndk.so"),
    dict(pkg="vendor.sprd.hardware.network", ifaces={"INetworkControl": ["default"]},
         lib="vendor.sprd.hardware.network-V1-ndk.so"),
    dict(pkg="vendor.sprd.hardware.soter", ifaces={"ISoter": ["default"]},
         lib="vendor.sprd.hardware.soter-V1-ndk.so"),
    dict(pkg="vendor.sprd.hardware.trusty", ifaces={"ITrustyClientProvider": ["default"]},
         lib="vendor.sprd.hardware.trusty-V1-ndk.so"),
    dict(pkg="vendor.sprd.hardware.tui", ifaces={"ITui": ["default"]},
         lib="vendor.sprd.hardware.tui-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.ai_engine", ifaces={"IAIEngineControl": ["default"]}, lib=None),
    dict(pkg="vendor.unisoc.hardware.power", ifaces={"IVendorPower": ["default"]},
         lib="vendor.unisoc.hardware.power-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio", ifaces={"IExtRadio": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.data", ifaces={"IExtRadioData": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio.data-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.ims", ifaces={"IImsRadio": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio.ims-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.lite",
         ifaces={"ILiteRadio": ["liteservice1", "liteservice2"]},
         lib="vendor.unisoc.hardware.radio.lite-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.messaging", ifaces={"IExtRadioMessaging": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio.messaging-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.modem", ifaces={"IExtRadioModem": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio.modem-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.network", ifaces={"IExtRadioNetwork": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio.network-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.sim", ifaces={"IExtRadioSim": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio.sim-V1-ndk.so"),
    dict(pkg="vendor.unisoc.hardware.radio.voice", ifaces={"IExtRadioVoice": ["slot1", "slot2"]},
         lib="vendor.unisoc.hardware.radio.voice-V1-ndk.so"),
]

# Stub services. "hidl"/"aidl" select entries by 1-based index into the
# inventories above (kept explicit so a service can be split later).
SERVICES = [
    dict(kind="hidl", dir="hidl/boot", name="vendor.sprd.hardware.boot@1.2-service.stub",
         rc_name="vendor.sprd.boot-hal-1-2", pick=[0]),
    dict(kind="hidl", dir="hidl/sprd-core", name="vendor.sprd.hardware-service.stub",
         rc_name="vendor.sprd.hidl-stub", pick=list(range(1, 20))),
    dict(kind="hidl", dir="hidl/swfingerprint", name="vendor.sw.swfingerprint@1.0-service.stub",
         rc_name="vendor.swfingerprint-stub", pick=[20]),
    dict(kind="aidl", dir="aidl/sprd-core", name="vendor.sprd.hardware-aidl-service.stub",
         rc_name="vendor.sprd.aidl-stub", pick=list(range(0, 11))),
    dict(kind="aidl", dir="aidl/unisoc-core", name="vendor.unisoc.hardware-aidl-service.stub",
         rc_name="vendor.unisoc.aidl-stub", pick=[11, 12]),
    dict(kind="aidl", dir="aidl/unisoc-radio", name="vendor.unisoc.hardware.radio-service.stub",
         rc_name="vendor.unisoc.radio-stub", pick=list(range(13, 22))),
]


def write(rel, text):
    path = ROOT / rel
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def hidl_ns(pkg, ver):
    return "::" + pkg.replace(".", "::") + "::V" + ver.replace(".", "_")


def hidl_include(pkg, ver, iface):
    return pkg.replace(".", "/") + "/" + ver + "/" + iface + ".h"


def aidl_path(pkg):
    return pkg.replace(".", "/")


def aidl_bn(iface):
    return "Bn" + iface[1:]


def gen_hidl_interfaces():
    roots = ["subdirs = [", '    "*"', "]", ""]
    for name, path, _tree in HIDL_ROOTS:
        roots += ["hidl_package_root {", '    name: "%s",' % name, '    path: "%s",' % path, "}", ""]
    write("interfaces/Android.bp", LICENSE_C + "\n" + "\n".join(roots))

    for item in HIDL:
        base = "interfaces/%s/%s" % (item["tree"], item["dir"])
        deps = ["android.hidl.base@1.0"]
        if item["ext"]:
            deps.append(item["ext_lib"])
        bp = ["hidl_interface {", '    name: "%s@%s",' % (item["pkg"], item["ver"]),
              '    root: "%s",' % item["root"], "    system_ext_specific: true,", "    srcs: ["]
        bp += ['        "%s.hal",' % i for i in item["ifaces"]]
        bp += ["    ],", "    interfaces: ["]
        bp += ['        "%s",' % d for d in deps]
        bp += ["    ],", "    gen_java: true,", "}", ""]
        write(base + "/Android.bp", LICENSE_C + "\n" + "\n".join(bp))

        for iface in item["ifaces"]:
            lines = [LICENSE_HAL.rstrip(), "",
                     "package %s@%s;" % (item["pkg"], item["ver"]), ""]
            if item["ext"] and iface in item["ext"]:
                lines += ["import %s;" % item["ext"], ""]
            lines += ["// Stub definition. The stock interface library exposes methods which are",
                      "// not modelled here; exact signatures are only available in the vendor",
                      "// interface source. See README.md and tools/dump_hal_api.sh.",
                      "// Known method names: " + (", ".join(item["methods"]) or "not recovered") + "."]
            if item["ext"] and iface in item["ext"]:
                lines += ["interface %s extends %s {" % (iface, item["ext"]), "};", ""]
            else:
                lines += ["interface %s {" % iface, "};", ""]
            write(base + "/" + iface + ".hal", "\n".join(lines))


DUMP_HEADER = """///////////////////////////////////////////////////////////////////////////////
// THIS FILE IS IMMUTABLE. DO NOT EDIT IN ANY CASE.                          //
///////////////////////////////////////////////////////////////////////////////

// This file is a snapshot of an AIDL file. Do not edit it manually. There are
// two cases:
// 1). this is a frozen version file - do not edit this in any case.
// 2). this is a 'current' file. If you make a backwards compatible change to
//     the interface (from the latest frozen version), the build system will
//     prompt you to update this file with `m <name>-update-api`.
//
// You must not make a backward incompatible change to any AIDL file built
// with the aidl_interface module type with versions property set. The module
// type is used to build AIDL files in a way that they can be used across
// independently updatable components of the system. If a device is shipped
// with such a backward incompatible change, it has a high risk of breaking
// later when a module using the interface is updated, e.g., Mainline modules.
"""


def gen_aidl_interfaces():
    bp = [LICENSE_C.rstrip(), ""]
    for item in AIDL:
        bp += ["aidl_interface {",
               '    name: "%s",' % item["pkg"],
               '    srcs: ["%s/*.aidl"],' % aidl_path(item["pkg"]),
               '    stability: "vintf",',
               '    owner: "%s",' % ("unisoc" if item["pkg"].startswith("vendor.unisoc") else "sprd"),
               "    vendor_available: true,",
               "    product_available: true,",
               "    backend: {",
               "        cpp: { enabled: false },",
               "        java: { enabled: false },",
               "        ndk: { enabled: true },",
               "    },",
               "}", ""]
    write("interfaces/aidl/Android.bp", "\n".join(bp))

    for item in AIDL:
        for iface in item["ifaces"]:
            rel = "%s/%s.aidl" % (aidl_path(item["pkg"]), iface)
            body = [LICENSE_HAL.rstrip(), "",
                    "package %s;" % item["pkg"], "",
                    "@VintfStability",
                    "interface %s {" % iface,
                    "    // Stub definition: the stock vendor interface has methods which are",
                    "    // not modelled here. Recover them with tools/dump_hal_api.sh before",
                    "    // replacing the stock HAL service on a shipping device.",
                    "}", ""]
            src = "\n".join(body)
            write("interfaces/aidl/" + rel, src)
            dump = LICENSE_HAL.rstrip() + "\n" + DUMP_HEADER + "\n" + \
                   "package %s;\n@VintfStability\ninterface %s {\n}\n" % (item["pkg"], iface)
            write("interfaces/aidl/aidl_api/%s/current/%s" % (item["pkg"], rel), dump)


def gen_hidl_service(svc):
    items = [HIDL[i] for i in svc["pick"]]
    name = svc["name"]
    base = svc["dir"]

    includes, aliases, structs, regs, libs = [], [], [], [], []
    for n, item in enumerate(items):
        ns = "ns%d" % n
        includes += ['#include <%s>' % hidl_include(item["pkg"], item["ver"], iface)
                     for iface in item["ifaces"]]
        aliases.append("namespace %s = %s;" % (ns, hidl_ns(item["pkg"], item["ver"])))
        libs.append("%s@%s" % (item["pkg"], item["ver"]))
        for iface, instances in item["ifaces"].items():
            cls = "Stub_%s_%s" % (item["dir"].replace("/", "_").replace(".", "_"), iface)
            structs.append("struct %s : public %s::%s {};" % (cls, ns, iface))
            for inst in instances:
                regs += ["    {",
                         "        sp<%s::%s> service = new %s();" % (ns, iface, cls),
                         '        if (service->registerAsService("%s") != android::OK) {' % inst,
                         '            LOG(ERROR) << "Failed to register %s/%s";' % (iface, inst),
                         "            return 1;",
                         "        }",
                         "    }"]

    src = [LICENSE_C.rstrip(), "", '#define LOG_TAG "%s"' % name, "",
           "#include <android-base/logging.h>", "#include <hidl/HidlTransportSupport.h>", ""]
    src += sorted(set(includes)) + [""] + aliases + ["",
           "using android::sp;", "using android::hardware::configureRpcThreadpool;",
           "using android::hardware::joinRpcThreadpool;", ""] + structs + ["",
           "int main() {", "    configureRpcThreadpool(1, true /* callerWillJoin */);", ""] + regs + ["",
           "    joinRpcThreadpool();", "    return 0;  // should never get here", "}", ""]
    write(base + "/service.cpp", "\n".join(src))

    bp = [LICENSE_C.rstrip(), "", "cc_binary {", '    name: "%s",' % name,
          '    defaults: ["hidl_defaults"],', '    init_rc: ["%s.rc"],' % name,
          '    vintf_fragments: ["%s.xml"],' % name, "    vendor: true,",
          '    relative_install_path: "hw",', '    srcs: ["service.cpp"],', "    shared_libs: [",
          '        "libbase",', '        "libhidlbase",', '        "liblog",', '        "libutils",']
    bp += ['        "%s",' % l for l in sorted(set(libs))] + ["    ],", "}", ""]
    write(base + "/Android.bp", "\n".join(bp))

    write(base + "/" + name + ".rc",
          "service %s /vendor/bin/hw/%s\n    class hal\n    user system\n    group system\n"
          % (svc["rc_name"], name))

    frag = ['<manifest version="1.0" type="device">']
    for item in items:
        frag += ['    <hal format="hidl">', '        <name>%s</name>' % item["pkg"],
                 '        <transport>hwbinder</transport>',
                 '        <version>%s</version>' % item["ver"],
                 '        <interface>']
        for iface, instances in item["ifaces"].items():
            if not instances:
                continue
            frag += ['            <name>%s</name>' % iface]
            frag += ['            <instance>%s</instance>' % i for i in instances]
        frag += ['        </interface>', '    </hal>']
    frag += ["</manifest>", ""]
    write(base + "/" + name + ".xml", "\n".join(frag))


def gen_aidl_service(svc):
    items = [AIDL[i] for i in svc["pick"]]
    name = svc["name"]
    base = svc["dir"]

    includes, usings, structs, regs, libs = [], [], [], [], []
    for item in items:
        libs.append(item["pkg"] + "-V1-ndk")
        ns = "::aidl::" + item["pkg"].replace(".", "::")
        for iface, instances in item["ifaces"].items():
            bn = aidl_bn(iface)
            cls = "Stub_" + aidl_path(item["pkg"]).replace("/", "_").replace(".", "_") + "_" + iface
            includes.append('#include <aidl/%s/%s.h>' % (aidl_path(item["pkg"]), bn))
            usings.append("using %s::%s;" % (ns, bn))
            structs.append("class %s : public %s {};" % (cls, bn))
            for inst in instances:
                regs += ["    {",
                         "        auto hal = ndk::SharedRefBase::make<%s>();" % cls,
                         "        const std::string instance = std::string(%s::descriptor) + \"/%s\";" % (cls, inst),
                         "        binder_status_t status =",
                         "                AServiceManager_addService(hal->asBinder().get(), instance.c_str());",
                         '        CHECK_EQ(status, STATUS_OK) << instance;',
                         "    }"]

    src = [LICENSE_C.rstrip(), "", '#define LOG_TAG "%s"' % name, "",
           "#include <android-base/logging.h>", "#include <android/binder_interface_utils.h>",
           "#include <android/binder_manager.h>", "#include <android/binder_process.h>",
           "#include <android/binder_status.h>", "#include <cstdlib>", "#include <string>", ""]
    src += sorted(set(includes)) + [""] + sorted(set(usings)) + [""] + structs + ["",
           "int main() {", "    ABinderProcess_setThreadPoolMaxThreadCount(0);", ""] + regs + ["",
           "    ABinderProcess_joinThreadPool();", "    return EXIT_FAILURE;  // should not reach", "}", ""]
    write(base + "/service.cpp", "\n".join(src))

    bp = [LICENSE_C.rstrip(), "", "cc_binary {", '    name: "%s",' % name,
          '    init_rc: ["%s.rc"],' % name,
          '    vintf_fragments: ["%s.xml"],' % name, "    vendor: true,",
          '    relative_install_path: "hw",', '    srcs: ["service.cpp"],', "    shared_libs: [",
          '        "libbase",', '        "libbinder_ndk",', '        "liblog",']
    bp += ['        "%s",' % l for l in sorted(set(libs))] + ["    ],", "}", ""]
    write(base + "/Android.bp", "\n".join(bp))

    write(base + "/" + name + ".rc",
          "service %s /vendor/bin/hw/%s\n    class hal\n    user system\n    group system\n"
          % (svc["rc_name"], name))

    frag = ['<manifest version="2.0" type="device">']
    for item in items:
        frag += ['    <hal format="aidl">', '        <name>%s</name>' % item["pkg"]]
        for iface, instances in item["ifaces"].items():
            for inst in instances:
                frag.append('        <fqname>%s/%s</fqname>' % (iface, inst))
        frag.append('    </hal>')
    frag += ["</manifest>", ""]
    write(base + "/" + name + ".xml", "\n".join(frag))


def gen_skeleton():
    write("Android.bp", LICENSE_C + "\nsoong_namespace {\n}\n")
    write("Android.mk", "# SPDX-License-Identifier: Apache-2.0\n\n"
          "ifeq ($(BOARD_HAS_SPRD_HARDWARE),true)\n"
          "include $(call all-subdir-makefiles)\n"
          "endif\n")
    write("interfaces/vendor_hal_makefile_generator.sh",
          "#!/bin/sh\n"
          "# SPDX-FileCopyrightText: The LineageOS Project\n"
          "# SPDX-License-Identifier: Apache-2.0\n"
          "#\n"
          "# Generate blueprint/makefiles for the vendor HALs.\n"
          'set -e\n'
          'cd "$(dirname "$0")/.."\n'
          'exec python3 tools/generate_hal_stubs.py "$@"\n')
    write("tools/dump_hal_api.sh", r'''#!/bin/sh
#
# SPDX-FileCopyrightText: The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#
# Print the Bp* method signatures exported by the stock private HAL interface
# libraries. Point BLOBS at an unpacked vendor image, e.g.
#   BLOBS=/home/hema/Workspace/e5/vendor tools/dump_hal_api.sh

BLOBS="${BLOBS:-/home/hema/Workspace/e5/vendor}"

find "$BLOBS" -type f \( -name 'vendor.sprd.hardware*@*.so' \
     -o -name 'vendor.sprd.hardware*-V1-ndk.so' \
     -o -name 'vendor.unisoc.hardware*-V1-ndk.so' \) \
     -path '*/lib64/*' | sort -u | while read -r lib; do
    printf '### %s\n' "$(basename "$lib")"
    nm -D --defined-only "$lib" 2>/dev/null | c++filt | \
        sed -nE 's/^.*::Bp(Hw)?([A-Za-z0-9_]+)::([A-Za-z0-9_]+)\((.*)$/\2::\3(\4/p' | \
        grep -vE '::(Bp|~|_hidl_|getInterfaceVersion|getInterfaceHash)' | sort -u
done
''')


def gen_readme():
    lines = [LICENSE_C.rstrip(), "", "# Common hardware components for UNISOC / Spreadtrum devices", "",
             "Source-side definitions of the vendor-private HAL interfaces found on the RongYue E5",
             "(`ums9158_1h10` / `ums9621`, Android 13 / LineageOS 20), plus no-op stub services that",
             "can replace their stock counterparts one by one.", "",
             "## Status", "",
             "This repository is **not wired into any product yet**. It is deliberately independent",
             "of `device/rongyue/e5` and of the repo manifest. Nothing here changes the current",
             "boot behaviour of the device.", "",
             "The interface declarations use the exact package / interface / instance names from",
             "`vendor/product/etc/vintf/compatibility_matrix.xml` and `vendor/etc/vintf/manifest*.xml`",
             "of the stock image, but their **method bodies are empty**. The stock prebuilt interface",
             "libraries do not ship `.hal` / `.aidl` sources, and C++ mangling does not encode return",
             "types, so the real transaction signatures cannot be reconstructed losslessly. Before",
             "switching a real client over to a stub, fill in its methods using",
             "`tools/dump_hal_api.sh` and verify the transaction codes.", "",
             "## Layout", "",
             "| Path | Purpose |", "| --- | --- |",
             "| `interfaces/` | `hidl_package_root` declarations for the two vendor roots |",
             "| `interfaces/hidl/` | HIDL `.hal` definitions, one directory per package/version |",
             "| `interfaces/hidl_sw/` | `vendor.sw.swfingerprint` HIDL definitions |",
             "| `interfaces/aidl/` | `aidl_interface` modules, `.aidl` sources and `aidl_api/current` dumps |",
             "| `hidl/` | binderized HIDL stub services, init rc and VINTF fragments |",
             "| `aidl/` | AIDL stub services, init rc and VINTF fragments |",
             "| `tools/` | inventory generator and blob API dumper |", "",
             "The build is gated by `BOARD_HAS_SPRD_HARDWARE`, mirroring",
             "`hardware/mediatek`'s `BOARD_HAS_MTK_HARDWARE`.", "",
             "## HIDL inventory", "",
             "| Package | Interfaces (instances) | Stock interface library |",
             "| --- | --- | --- |"]
    for item in HIDL:
        ifaces = ", ".join("%s (%s)" % (i, "/".join(v) or "callback") for i, v in item["ifaces"].items())
        lines.append("| `%s@%s` | %s | %s |" %
                     (item["pkg"], item["ver"], ifaces, "`" + item["lib"] + "`" if item["lib"] else "n/a"))
    lines += ["", "## AIDL inventory", "",
              "| Package | Interfaces (instances) | Stock interface library |", "| --- | --- | --- |"]
    for item in AIDL:
        ifaces = ", ".join("%s (%s)" % (i, "/".join(v)) for i, v in item["ifaces"].items())
        lines.append("| `%s` | %s | %s |" %
                     (item["pkg"], ifaces, "`" + item["lib"] + "`" if item["lib"] else "n/a"))
    lines += ["", "## Stub services", "",
              "| Module | Registers |", "| --- | --- |"]
    for svc in SERVICES:
        if svc["kind"] == "hidl":
            desc = ", ".join("%s@%s" % (HIDL[i]["pkg"], HIDL[i]["ver"]) for i in svc["pick"])
        else:
            desc = ", ".join(AIDL[i]["pkg"] for i in svc["pick"])
        lines.append("| `%s` | %s |" % (svc["name"], desc))
    lines += ["", "## Integration (not applied)", "",
              "```make", "# BoardConfig.mk", "BOARD_HAS_SPRD_HARDWARE := true", "",
              "# device.mk", "PRODUCT_SOONG_NAMESPACES += hardware/sprd", "PRODUCT_PACKAGES += \\",
              "    vendor.sprd.hardware.boot@1.2-service.stub \\",
              "    vendor.sprd.hardware-service.stub \\",
              "    vendor.sprd.hardware-aidl-service.stub \\",
              "    vendor.unisoc.hardware-aidl-service.stub \\",
              "    vendor.unisoc.hardware.radio-service.stub", "```", "",
              "## Conflicts with the stock blobs", "",
              "The stock image already installs interface libraries with the same names",
              "(`vendor.sprd.hardware.*@1.x.so`, `*-impl.so`, `*-V1-ndk.so`) and runs matching",
              "services from `vendor/bin/hw`. Enabling a stub service therefore requires, for each",
              "HAL:", "",
              "1. removing or disabling the corresponding `vendor/etc/init/*.rc` entry and",
              "   `vendor/bin/hw/*-service` from `proprietary-files.txt`;",
              "2. keeping the stock interface library out of the vendor partition, or making the",
              "   source-built one win the linker namespace;",
              "3. deleting the hand-written declaration from",
              "   `device/rongyue/e5/vintf/manifest.xml` so it comes from the stub's",
              "   `vintf_fragments` instead;",
              "4. removing the matching entry from `framework_compatibility_matrix.xml` if the",
              "   HAL is no longer optional.", "",
              "The AIDL interfaces are unfrozen (`owner` set, no `versions`), so the generated",
              "NDK library is `<name>-V1-ndk`. Freeze them with `m <name>-freeze-api` once the",
              "method set is final.", "",
              "## Regenerating", "",
              "```sh", "python3 tools/generate_hal_stubs.py",
              "BLOBS=/home/hema/Workspace/e5/vendor tools/dump_hal_api.sh", "```", ""]
    write("README.md", "\n".join(lines))


def main():
    global ROOT
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True,
                        help="scratch output directory (must not be hardware/sprd)")
    args = parser.parse_args()
    if args.output.resolve().is_relative_to(ROOT):
        parser.error("refusing to overwrite maintained HALs and build files")
    if args.output.exists() and (not args.output.is_dir() or any(args.output.iterdir())):
        parser.error("output must be a new or empty scratch directory")
    ROOT = args.output.resolve()
    gen_skeleton()
    gen_hidl_interfaces()
    gen_aidl_interfaces()
    for svc in SERVICES:
        (gen_hidl_service if svc["kind"] == "hidl" else gen_aidl_service)(svc)
    gen_readme()
    print("generated hardware/sprd from %d HIDL and %d AIDL packages" % (len(HIDL), len(AIDL)))


if __name__ == "__main__":
    main()
