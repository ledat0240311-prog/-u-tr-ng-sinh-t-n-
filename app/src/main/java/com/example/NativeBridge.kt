package com.example

object NativeBridge {
    init {
        try {
            System.loadLibrary("lastsurvivor")
        } catch (e: UnsatisfiedLinkError) {
            e.printStackTrace()
        }
    }

    @JvmStatic
    external fun nativeInit(width: Int, height: Int)

    @JvmStatic
    external fun nativeResize(width: Int, height: Int)

    @JvmStatic
    external fun nativeStep()

    @JvmStatic
    external fun nativeOnTouchMove(x: Float, y: Float, sprint: Boolean)

    @JvmStatic
    external fun nativeOnTouchLook(dx: Float, dy: Float)

    @JvmStatic
    external fun nativeOnFire(pressed: Boolean)

    @JvmStatic
    external fun nativeOnAim(aiming: Boolean)

    @JvmStatic
    external fun nativeOnReload()

    @JvmStatic
    external fun nativeOnJump()

    @JvmStatic
    external fun nativeOnCrouch()

    @JvmStatic
    external fun nativeOnSwitchWeapon(slot: Int)

    @JvmStatic
    external fun nativeApplyUpgrade(upgradeType: Int): Boolean

    @JvmStatic
    external fun nativeRestart()

    @JvmStatic
    external fun nativeGetHUDData(outFloats: FloatArray, outStrings: Array<String>)

    @JvmStatic
    external fun nativeDrainAudioEvents(outEvents: IntArray): Int
}
