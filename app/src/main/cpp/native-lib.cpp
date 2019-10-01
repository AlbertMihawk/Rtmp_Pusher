#include <jni.h>
#include <string>

//内部已经添加extern "C"
//extern "C" {
#include <rtmp.h>
#include <x264.h>
#include <pthread.h>
#include "VideoChannel.h"
#include "safe_queue.h"
#include "marco.h"
//}

VideoChannel *videoChannel = 0;
SafeQueue<RTMPPacket *> packets;
bool isStart;
bool readyPushing;
pthread_t pid_start;
uint32_t start_time;

void releasePackets(RTMPPacket **packet) {
    if (packet) {
        RTMPPacket_Free(*packet);
        delete packet;
        packet = 0;
    }
}

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
extern "C"
JNIEXPORT void JNICALL
Java_com_albert_rtmp_1pusher_NEPusher_native_1init(JNIEnv *env, jobject thiz) {
    //编码器进行编码 工具类 VideoChannel
    videoChannel = new VideoChannel;

    //准备一个安全队列，把数据放入队列，在同一的线程中取出数据，在发送给服务器
    packets.setReleaseCallback(releasePackets);

}

//启动子线程任务
void *task_start(void *args) {
    char *url = static_cast<char *>(args);
    RTMP *rtmp = 0;
    int ret = 0;
    do {
        //1、rtmp 初始化
        rtmp = RTMP_Alloc();
        if (!rtmp) {
            LOGE("rtmp 初始化失败");
            break;
        }
        //1.1、初始化
        RTMP_Init(rtmp);
        rtmp->Link.timeout = 5;

        //2、设置流媒体失败
        ret = RTMP_SetupURL(rtmp, url);
        if (!ret) {
            LOGE("设置流媒体url失败");
            break;
        }

        //3、开启输出模式
        RTMP_EnableWrite(rtmp);
        //4、建立连接
        ret = RTMP_Connect(rtmp, 0);
        if (!ret) {
            LOGE("建立连接失败");
            break;
        }
        //5、建立连接流
        ret = RTMP_ConnectStream(rtmp, 0);
        if (!ret) {
            LOGE("建立连接流失败");
            break;
        }
        //以上5步，已经准备好了
        readyPushing = 1;

        //6、记录开始推流的时间
        start_time = RTMP_GetTime();
        //后面要对安全队列进行取数据的操作
        packets.setWork(1);
        RTMPPacket *packet = 0;
        //循环从队列中取数据（rtmp包),然后发送
        while (readyPushing) {
            packets.pop(packet);
            if (!readyPushing) {
                break;
            }
            if (!packet) {
                continue;
            }
            //成功渠道数据包，发送
            packet->m_nInfoField2 = rtmp->m_stream_id;
            //将true放入队列
            ret = RTMP_SendPacket(rtmp, packet, 1);
            if (!ret) {
                LOGE("rtmp 断开");
                break;
            }
        }
        releasePackets(&packet);

    } while (0);

    isStart = 0;
    if (rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        delete rtmp;
    }

    delete url;
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_albert_rtmp_1pusher_NEPusher_native_1start(JNIEnv *env, jobject thiz, jstring path_) {
    if (isStart) {
        return;
    }
    isStart = 1;
    const char *path = env->GetStringUTFChars(path_, 0);
    //Flag 来控制(isPlaying)
    char *url = new char(strlen(path) + 1);
    strcpy(url, path);
    //创建线程来进行直播
    pthread_create(&pid_start, 0, task_start, &url);

    env->ReleaseStringUTFChars(path_, path);

}
extern "C"
JNIEXPORT void JNICALL
Java_com_albert_rtmp_1pusher_NEPusher_native_1stop(JNIEnv *env, jobject thiz) {
    isStart = 0;
    readyPushing = 0;
    packets.setWork(0);
    //等着子线程执行完
    pthread_join(pid_start, 0);

}