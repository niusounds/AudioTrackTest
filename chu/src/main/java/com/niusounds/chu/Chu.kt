package com.niusounds.chu

import java.nio.ByteBuffer

class Chu {
    private var state: Long = 0

    fun initPcm16(bufferSizeInBytes: Int, overlap: Float) {
        release()
        state = nativeInit(bufferSizeInBytes / 2, overlap)
    }

    fun release() {
        if (state != 0L) {
            nativeRelease(state)
            state = 0L
        }
    }

    fun process(buffer: ByteBuffer) {
        if (state != 0L) {
            nativeProcess(state, buffer)
        }
    }

    private external fun nativeInit(bufsize: Int, overlap: Float): Long
    private external fun nativeRelease(statePtr: Long)
    private external fun nativeProcess(statePtr: Long, buffer: ByteBuffer)

    companion object {
        init {
            System.loadLibrary("chu")
        }
    }
}