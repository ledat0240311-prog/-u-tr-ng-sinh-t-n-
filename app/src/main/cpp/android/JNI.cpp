#include <jni.h>
#include "Game.h"
#include "GameLoop.h"
#include "AudioManager.h"
#include "HUD.h"
#include <android/log.h>

#define JNI_METHOD(return_type, method_name) \
  JNIEXPORT return_type JNICALL Java_com_example_NativeBridge_##method_name

extern "C" {

JNI_METHOD(void, nativeInit)(JNIEnv* env, jclass clazz, jint width, jint height) {
    LastSurvivor::GameLoop::Instance().Init(width, height);
}

JNI_METHOD(void, nativeResize)(JNIEnv* env, jclass clazz, jint width, jint height) {
    LastSurvivor::Game::Instance().Resize(width, height);
}

JNI_METHOD(void, nativeStep)(JNIEnv* env, jclass clazz) {
    LastSurvivor::GameLoop::Instance().Step();
}

JNI_METHOD(void, nativeOnTouchMove)(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jboolean sprint) {
    LastSurvivor::Game::Instance().HandleMove(x, y, sprint == JNI_TRUE);
}

JNI_METHOD(void, nativeOnTouchLook)(JNIEnv* env, jclass clazz, jfloat dx, jfloat dy) {
    LastSurvivor::Game::Instance().HandleLook(dx, dy);
}

JNI_METHOD(void, nativeOnFire)(JNIEnv* env, jclass clazz, jboolean pressed) {
    LastSurvivor::Game::Instance().HandleFire(pressed == JNI_TRUE);
}

JNI_METHOD(void, nativeOnAim)(JNIEnv* env, jclass clazz, jboolean aiming) {
    LastSurvivor::Game::Instance().HandleAim(aiming == JNI_TRUE);
}

JNI_METHOD(void, nativeOnReload)(JNIEnv* env, jclass clazz) {
    LastSurvivor::Game::Instance().HandleReload();
}

JNI_METHOD(void, nativeOnJump)(JNIEnv* env, jclass clazz) {
    LastSurvivor::Game::Instance().HandleJump();
}

JNI_METHOD(void, nativeOnCrouch)(JNIEnv* env, jclass clazz) {
    LastSurvivor::Game::Instance().HandleCrouch();
}

JNI_METHOD(void, nativeOnSwitchWeapon)(JNIEnv* env, jclass clazz, jint slot) {
    LastSurvivor::Game::Instance().HandleSwitchWeapon(slot);
}

JNI_METHOD(jboolean, nativeApplyUpgrade)(JNIEnv* env, jclass clazz, jint upgradeType) {
    bool ok = LastSurvivor::Game::Instance().ApplyUpgrade(upgradeType);
    return ok ? JNI_TRUE : JNI_FALSE;
}

JNI_METHOD(void, nativeRestart)(JNIEnv* env, jclass clazz) {
    LastSurvivor::Game::Instance().Restart();
}

JNI_METHOD(void, nativeGetHUDData)(JNIEnv* env, jclass clazz, jfloatArray outFloats, jobjectArray outStrings) {
    LastSurvivor::HUDData data;
    LastSurvivor::Game::Instance().GetHUDData(data);

    // Pack into float array
    jfloat buffer[18];
    buffer[0] = data.hp;
    buffer[1] = data.maxHp;
    buffer[2] = data.armor;
    buffer[3] = data.maxArmor;
    buffer[4] = data.stamina;
    buffer[5] = data.maxStamina;
    buffer[6] = static_cast<float>(data.currentAmmo);
    buffer[7] = static_cast<float>(data.reserveAmmo);
    buffer[8] = static_cast<float>(data.weaponSlot);
    buffer[9] = static_cast<float>(data.waveNumber);
    buffer[10] = static_cast<float>(data.enemiesRemaining);
    buffer[11] = static_cast<float>(data.totalKills);
    buffer[12] = static_cast<float>(data.score);
    buffer[13] = static_cast<float>(data.coins);
    buffer[14] = static_cast<float>(data.waveState);
    buffer[15] = data.stateTimer;
    buffer[16] = static_cast<float>(data.hitmarkerActive);
    buffer[17] = data.damageVignette;

    env->SetFloatArrayRegion(outFloats, 0, 18, buffer);

    if (outStrings != nullptr && env->GetArrayLength(outStrings) > 0) {
        jstring wName = env->NewStringUTF(data.weaponName);
        env->SetObjectArrayElement(outStrings, 0, wName);
        env->DeleteLocalRef(wName);
    }
}

JNI_METHOD(jint, nativeDrainAudioEvents)(JNIEnv* env, jclass clazz, jintArray outEvents) {
    jsize len = env->GetArrayLength(outEvents);
    int buffer[32];
    int count = LastSurvivor::AudioManager::Instance().DrainEvents(buffer, len > 32 ? 32 : len);
    if (count > 0) {
        env->SetIntArrayRegion(outEvents, 0, count, reinterpret_cast<const jint*>(buffer));
    }
    return count;
}

} // extern "C"
