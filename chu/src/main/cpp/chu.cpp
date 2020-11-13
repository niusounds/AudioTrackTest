#include <jni.h>
#include <string>
#include <vector>

#define spl0 samples[i]
#define spl1 samples[i+1]

class NativeChu {
private:
    int bufsize;
    int hbsz;
    int qbsz;
    short *buffer;
    int recpos;
    int recbuf;
    int playpos;
    int playbuf;
    int o_start;
    int o_end;
    double fadesc;

public:
    explicit NativeChu(int bufsize, float overlap) {
        this->bufsize = bufsize;
        hbsz = bufsize / 2;
        qbsz = bufsize / 4;
        buffer = new short[bufsize * 4];
        for (int i = 0; i < bufsize * 4; ++i) {
            buffer[i] = 0;
        }
        recpos = 0;
        recbuf = bufsize;
        playpos = hbsz;
        playbuf = 0;
        o_start = qbsz * (2 - overlap);
        o_end = qbsz * (2 + overlap);
        fadesc = 1.0f / (hbsz - o_start);
    }

    ~NativeChu() {
        delete buffer;
    }

    void process(short *samples, int size) {
        for (int i = 0; i < size; i += 2) {
            buffer[recbuf + recpos * 2] = spl0;
            buffer[recbuf + recpos * 2 + 1] = spl1;

            if (playpos < o_start) {
                // first quarter (first half of first block)
                spl0 = buffer[playbuf + playpos * 4];
                spl1 = buffer[playbuf + playpos * 4 + 1];
            } else if (playpos < hbsz) {
                // second quarter
                double sc = (playpos - o_start) * fadesc;
                spl0 =
                        // second half of first block mix
                        buffer[playbuf + playpos * 4] * (1 - sc) +
                        // first half of second block mix
                        buffer[playbuf + (playpos - qbsz) * 4] * sc;
                spl1 =
                        // second half of first block mix
                        buffer[playbuf + playpos * 4 + 1] * (1 - sc) +
                        // first half of second block mix
                        buffer[playbuf + (playpos - qbsz) * 4 + 1] * sc;
            } else if (playpos < o_end) {
                // third quarter
                double sc = (playpos - hbsz) * fadesc;
                spl0 =
                        // second half of second block
                        buffer[playbuf + (playpos - qbsz) * 4] * (1 - sc) +
                        // first half of third block
                        buffer[playbuf + (playpos - hbsz) * 4] * sc;
                spl1 =
                        // second half of second block
                        buffer[playbuf + (playpos - qbsz) * 4 + 1] * (1 - sc) +
                        // first half of third block
                        buffer[playbuf + (playpos - hbsz) * 4 + 1] * sc;
            } else {
                // last quarter (second half of third block)
                spl0 = buffer[playbuf + (playpos - hbsz) * 4];
                spl1 = buffer[playbuf + (playpos - hbsz) * 4 + 1];
            }

            recpos += 1;
            if (recpos >= bufsize) {
                recpos = 0;
                recbuf = !recbuf * bufsize * 2;
            }

            playpos += 1;
            if (playpos >= bufsize) {
                playpos = 0;
                playbuf = !playbuf * bufsize * 2;
            }
        }
    }
};

extern "C" {

JNIEXPORT jlong JNICALL
Java_com_niusounds_chu_Chu_nativeInit(JNIEnv *env,
                                      jobject thiz,
                                      jint bufsize,
                                      jfloat overlap) {
    return reinterpret_cast<jlong>(new NativeChu(bufsize, overlap));
}

JNIEXPORT void JNICALL
Java_com_niusounds_chu_Chu_nativeRelease(JNIEnv *env,
                                         jobject thiz,
                                         jlong statePtr) {
    auto *state = reinterpret_cast<NativeChu *>( statePtr);
    delete state;
}

JNIEXPORT void JNICALL
Java_com_niusounds_chu_Chu_nativeProcess(JNIEnv *env,
                                         jobject thiz,
                                         jlong statePtr,
                                         jobject buffer) {
    auto *state = reinterpret_cast<NativeChu *>( statePtr);

    auto *sbuffer = static_cast<short *>(env->GetDirectBufferAddress(buffer));
    jlong len = env->GetDirectBufferCapacity(buffer) / 2;
    state->process(sbuffer, len);
}

}
