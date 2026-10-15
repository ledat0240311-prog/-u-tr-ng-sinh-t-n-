package com.example.ui

import android.annotation.SuppressLint
import android.content.Context
import android.opengl.GLSurfaceView
import android.view.MotionEvent
import com.example.NativeBridge
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

@SuppressLint("ViewConstructor")
class GameGLSurfaceView(
    context: Context,
    private val onLookSwipe: (Float, Float) -> Unit
) : GLSurfaceView(context) {

    private var lastTouchX = 0f
    private var lastTouchY = 0f
    private var isDraggingLook = false
    private var activeLookPointerId = -1

    init {
        setEGLContextClientVersion(3)
        setRenderer(object : Renderer {
            override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
                // Handled in onSurfaceChanged with concrete dimensions
            }

            override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
                NativeBridge.nativeInit(width, height)
                NativeBridge.nativeResize(width, height)
            }

            override fun onDrawFrame(gl: GL10?) {
                NativeBridge.nativeStep()
            }
        })
        renderMode = RENDERMODE_CONTINUOUSLY
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        val width = width.toFloat()

        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                val index = event.actionIndex
                val x = event.getX(index)
                val y = event.getY(index)

                // Only right 60% of screen controls camera orientation
                if (x > width * 0.40f && !isDraggingLook) {
                    activeLookPointerId = event.getPointerId(index)
                    lastTouchX = x
                    lastTouchY = y
                    isDraggingLook = true
                    return true
                }
            }

            MotionEvent.ACTION_MOVE -> {
                if (isDraggingLook) {
                    val pointerIndex = event.findPointerIndex(activeLookPointerId)
                    if (pointerIndex >= 0) {
                        val currentX = event.getX(pointerIndex)
                        val currentY = event.getY(pointerIndex)
                        val dx = (currentX - lastTouchX) * 0.22f
                        val dy = (lastTouchY - currentY) * 0.22f

                        NativeBridge.nativeOnTouchLook(dx, dy)
                        onLookSwipe(dx, dy)

                        lastTouchX = currentX
                        lastTouchY = currentY
                        return true
                    }
                }
            }

            MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                val index = event.actionIndex
                if (event.getPointerId(index) == activeLookPointerId) {
                    isDraggingLook = false
                    activeLookPointerId = -1
                    return true
                }
            }

            MotionEvent.ACTION_CANCEL -> {
                isDraggingLook = false
                activeLookPointerId = -1
            }
        }
        return super.onTouchEvent(event)
    }
}
