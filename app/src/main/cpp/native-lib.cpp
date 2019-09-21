#include <jni.h>
#include <string>

//内部已经添加extern "C"
//extern "C" {
#include <rtmp.h>
#include <x264.h>
//}

extern "C" JNIEXPORT jstring JNICALL
Java_com_albert_rtmp_1pusher_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    char version[50];
    sprintf(version, "rtmp version:%d", RTMP_LibVersion());

//    return env->NewStringUTF(hello.c_str());
    return env->NewStringUTF(version);
}
