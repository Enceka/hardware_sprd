// SPDX-FileCopyrightText: 2026 The LineageOS Project
// SPDX-License-Identifier: Apache-2.0

package vendor.unisoc.hardware.power;

@VintfStability
interface IVendorPower {
    oneway void acquirePowerHintBySceneId(in IBinder token, String tag, int sceneId);
    oneway void releasePowerHintBySceneId(in IBinder token, String tag, int sceneId);
    oneway void acquirePowerHintBySceneName(in IBinder token, String tag, String sceneName);
    oneway void releasePowerHintBySceneName(in IBinder token, String tag, String sceneName);
    int getSceneIdByName(String sceneName);
    void controlPowerHint(int enabled);
    int registerPowerHintScene(String tag, in String[] config);
    oneway void unregisterPowerHintScene(String tag, int sceneId);
    oneway void setVipTaskLevel(String tag, String type, int pid, int level);
}
