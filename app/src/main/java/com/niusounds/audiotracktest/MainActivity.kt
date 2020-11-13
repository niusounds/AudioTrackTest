package com.niusounds.audiotracktest

import android.media.*
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.niusounds.chu.Chu
import java.nio.ByteBuffer
import kotlin.concurrent.thread

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    private var writeThread: Thread? = null

    override fun onResume() {
        super.onResume()
        writeThread = thread {
            val sampleRate = 48000
            val channel = AudioFormat.CHANNEL_OUT_STEREO
            val format = AudioFormat.ENCODING_PCM_16BIT
            val bufferSize = 3840
            val track = AudioTrack(
                AudioManager.STREAM_MUSIC,
                sampleRate,
                channel,
                format,
                bufferSize,
                AudioTrack.MODE_STREAM
            )
            val record = AudioRecord(
                MediaRecorder.AudioSource.MIC,
                sampleRate,
                AudioFormat.CHANNEL_IN_STEREO,
                format,
                bufferSize,
            )

            val buffer = ByteBuffer.allocateDirect(bufferSize)
            track.play()
            record.startRecording()

            val vc = Chu()
            vc.initPcm16(bufferSize, 0.4f)

            while (!Thread.interrupted()) {
                buffer.clear()
                val readSize = record.read(buffer, bufferSize)

                vc.process(buffer)

                track.write(buffer, readSize, AudioTrack.WRITE_BLOCKING)
            }
            vc.release()

            track.stop()
            track.release()
        }
    }

    override fun onPause() {
        writeThread?.interrupt()
        writeThread = null
        super.onPause()
    }
}

