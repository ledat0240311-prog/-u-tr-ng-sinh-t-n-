package com.example.audio

import android.content.Context
import android.media.AudioAttributes
import android.media.AudioFormat
import android.media.AudioTrack
import android.os.Build
import android.os.VibrationEffect
import android.os.Vibrator
import android.os.VibratorManager
import java.io.ByteArrayOutputStream
import java.nio.ByteBuffer
import java.nio.ByteOrder
import kotlin.math.sin
import kotlin.random.Random

/**
 * High-performance procedural game audio manager.
 * Generates synthetic gunshot, reload, impact, and monster sound clips
 * in PCM16 format and caches them for zero-latency playback.
 */
class GameSoundManager(private val context: Context) {

    private val vibrator: Vibrator? = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
        val vm = context.getSystemService(Context.VIBRATOR_MANAGER_SERVICE) as? VibratorManager
        vm?.defaultVibrator
    } else {
        @Suppress("DEPRECATION")
        context.getSystemService(Context.VIBRATOR_SERVICE) as? Vibrator
    }

    private val sampleRate = 22050
    private val soundBuffers = HashMap<Int, ByteArray>()

    init {
        // Pre-generate procedural PCM waveforms
        soundBuffers[1] = generateGunshot(0.12f, 280f, 0.4f)  // Pistol
        soundBuffers[2] = generateGunshot(0.08f, 320f, 0.6f)  // SMG
        soundBuffers[3] = generateGunshot(0.14f, 220f, 0.5f)  // Rifle
        soundBuffers[4] = generateShotgunBlast()               // Shotgun
        soundBuffers[5] = generateSniperBoom()                // Sniper
        soundBuffers[6] = generateClickTone(1200f, 0.08f)     // Reload
        soundBuffers[7] = generateClickTone(400f, 0.04f)      // Empty click
        soundBuffers[8] = generateHurtNoise()                  // Player hurt
        soundBuffers[9] = generateFootstep()                   // Footstep
        soundBuffers[10] = generateDeathChord()                // Player death
        soundBuffers[11] = generateMonsterGrowl()              // Enemy attack
        soundBuffers[12] = generateHitThud()                   // Enemy hurt
        soundBuffers[13] = generateEnemyDeath()                // Enemy death
        soundBuffers[14] = generateChime(880f)                 // Loot pickup
        soundBuffers[15] = generateWaveFanfare()               // Wave start
        soundBuffers[16] = generateWaveFanfare(true)           // Wave cleared
        soundBuffers[17] = generateChime(1050f)                // Upgrade
    }

    fun playEvent(eventId: Int) {
        val pcm = soundBuffers[eventId] ?: return
        playPcmAsync(pcm)

        // Haptic feedback for major events
        if (eventId in 1..5) {
            triggerHaptic(if (eventId == 5 || eventId == 4) 40 else 20)
        } else if (eventId == 8) {
            triggerHaptic(60)
        }
    }

    private fun triggerHaptic(durationMs: Long) {
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                vibrator?.vibrate(
                    VibrationEffect.createOneShot(
                        durationMs,
                        VibrationEffect.DEFAULT_AMPLITUDE
                    )
                )
            } else {
                @Suppress("DEPRECATION")
                vibrator?.vibrate(durationMs)
            }
        } catch (_: Exception) {
        }
    }

    private fun playPcmAsync(data: ByteArray) {
        Thread {
            try {
                val minBuf = AudioTrack.getMinBufferSize(
                    sampleRate,
                    AudioFormat.CHANNEL_OUT_MONO,
                    AudioFormat.ENCODING_PCM_16BIT
                )
                val bufferSize = maxOf(minBuf, data.size)
                val track = AudioTrack.Builder()
                    .setAudioAttributes(
                        AudioAttributes.Builder()
                            .setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
                            .setUsage(AudioAttributes.USAGE_GAME)
                            .build()
                    )
                    .setAudioFormat(
                        AudioFormat.Builder()
                            .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                            .setSampleRate(sampleRate)
                            .setChannelMask(AudioFormat.CHANNEL_OUT_MONO)
                            .build()
                    )
                    .setBufferSizeInBytes(bufferSize)
                    .setTransferMode(AudioTrack.MODE_STATIC)
                    .build()

                track.write(data, 0, data.size)
                track.play()
                Thread.sleep((data.size.toFloat() / (sampleRate * 2) * 1000).toLong() + 50)
                track.stop()
                track.release()
            } catch (_: Exception) {
            }
        }.start()
    }

    private fun generateGunshot(duration: Float, baseFreq: Float, noiseMix: Float): ByteArray {
        val totalSamples = (duration * sampleRate).toInt()
        val stream = ByteArrayOutputStream(totalSamples * 2)
        val buf = ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)

        for (i in 0 until totalSamples) {
            val progress = i.toFloat() / totalSamples
            val decay = (1.0f - progress) * (1.0f - progress)
            val sine = sin(2.0 * Math.PI * (baseFreq * (1.0f - progress * 0.7f)) * i / sampleRate).toFloat()
            val noise = (Random.nextFloat() * 2.0f - 1.0f)
            val sample = (sine * (1.0f - noiseMix) + noise * noiseMix) * decay
            val pcmShort = (sample.coerceIn(-1.0f, 1.0f) * 32000).toInt().toShort()
            buf.putShort(0, pcmShort)
            stream.write(buf.array())
        }
        return stream.toByteArray()
    }

    private fun generateShotgunBlast(): ByteArray {
        val duration = 0.22f
        val totalSamples = (duration * sampleRate).toInt()
        val stream = ByteArrayOutputStream(totalSamples * 2)
        val buf = ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)

        for (i in 0 until totalSamples) {
            val progress = i.toFloat() / totalSamples
            val decay = (1.0f - progress).coerceAtLeast(0f)
            val noise = (Random.nextFloat() * 2.0f - 1.0f)
            val lowBoom = sin(2.0 * Math.PI * 120.0 * (1.0f - progress) * i / sampleRate).toFloat()
            val sample = (noise * 0.75f + lowBoom * 0.45f) * (decay * decay)
            val pcmShort = (sample.coerceIn(-1.0f, 1.0f) * 32500).toInt().toShort()
            buf.putShort(0, pcmShort)
            stream.write(buf.array())
        }
        return stream.toByteArray()
    }

    private fun generateSniperBoom(): ByteArray {
        val duration = 0.35f
        val totalSamples = (duration * sampleRate).toInt()
        val stream = ByteArrayOutputStream(totalSamples * 2)
        val buf = ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)

        for (i in 0 until totalSamples) {
            val progress = i.toFloat() / totalSamples
            val decay = (1.0f - progress)
            val noise = (Random.nextFloat() * 2.0f - 1.0f)
            val punch = sin(2.0 * Math.PI * 90.0 * (1.0f - progress * 0.5f) * i / sampleRate).toFloat()
            val sample = (punch * 0.6f + noise * 0.5f) * (decay * decay * decay)
            val pcmShort = (sample.coerceIn(-1.0f, 1.0f) * 32700).toInt().toShort()
            buf.putShort(0, pcmShort)
            stream.write(buf.array())
        }
        return stream.toByteArray()
    }

    private fun generateClickTone(freq: Float, duration: Float): ByteArray {
        val totalSamples = (duration * sampleRate).toInt()
        val stream = ByteArrayOutputStream(totalSamples * 2)
        val buf = ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)

        for (i in 0 until totalSamples) {
            val progress = i.toFloat() / totalSamples
            val decay = 1.0f - progress
            val sample = sin(2.0 * Math.PI * freq * i / sampleRate).toFloat() * decay
            buf.putShort(0, (sample.coerceIn(-1f, 1f) * 25000).toInt().toShort())
            stream.write(buf.array())
        }
        return stream.toByteArray()
    }

    private fun generateHurtNoise(): ByteArray {
        return generateGunshot(0.18f, 160f, 0.4f)
    }

    private fun generateFootstep(): ByteArray {
        return generateGunshot(0.04f, 80f, 0.5f)
    }

    private fun generateDeathChord(): ByteArray {
        return generateGunshot(0.5f, 90f, 0.2f)
    }

    private fun generateMonsterGrowl(): ByteArray {
        return generateGunshot(0.25f, 110f, 0.7f)
    }

    private fun generateHitThud(): ByteArray {
        return generateGunshot(0.08f, 150f, 0.5f)
    }

    private fun generateEnemyDeath(): ByteArray {
        return generateGunshot(0.28f, 130f, 0.6f)
    }

    private fun generateChime(freq: Float): ByteArray {
        val duration = 0.2f
        val totalSamples = (duration * sampleRate).toInt()
        val stream = ByteArrayOutputStream(totalSamples * 2)
        val buf = ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)

        for (i in 0 until totalSamples) {
            val progress = i.toFloat() / totalSamples
            val decay = 1.0f - progress
            val sample = sin(2.0 * Math.PI * freq * i / sampleRate).toFloat() * (decay * decay)
            buf.putShort(0, (sample.coerceIn(-1f, 1f) * 26000).toInt().toShort())
            stream.write(buf.array())
        }
        return stream.toByteArray()
    }

    private fun generateWaveFanfare(cleared: Boolean = false): ByteArray {
        val duration = 0.4f
        val totalSamples = (duration * sampleRate).toInt()
        val stream = ByteArrayOutputStream(totalSamples * 2)
        val buf = ByteBuffer.allocate(2).order(ByteOrder.LITTLE_ENDIAN)

        val f1 = if (cleared) 523.25f else 440f
        val f2 = if (cleared) 659.25f else 554f

        for (i in 0 until totalSamples) {
            val progress = i.toFloat() / totalSamples
            val decay = 1.0f - progress
            val freq = if (progress < 0.5f) f1 else f2
            val sample = sin(2.0 * Math.PI * freq * i / sampleRate).toFloat() * decay
            buf.putShort(0, (sample.coerceIn(-1f, 1f) * 28000).toInt().toShort())
            stream.write(buf.array())
        }
        return stream.toByteArray()
    }
}
