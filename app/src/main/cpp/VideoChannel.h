//
// Created by lixiaoxu on 2019-10-01.
//

#ifndef RTMP_PUSHER_VIDEOCHANNEL_H
#define RTMP_PUSHER_VIDEOCHANNEL_H


#include <rtmp.h>
#include <x264.h>
#include <pthread.h>
#include <cstring>
#include "macro.h"

class VideoChannel {
    typedef void (*VideoCallback)(RTMPPacket *packet);

public:
    VideoChannel();

    virtual ~VideoChannel();

    void initVideoEncoder(int w, int h, int fps, int bitrate);


    void encodeData(int8_t *data);

    void sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len);

    void setVideoCallback(VideoCallback videoCallback);

    void sendFrame(int type, int payload, uint8_t *pPayload);

private:
    int mWidth;
    int mHeight;
    int mFps;
    int mBitrate;
    int y_len;
    int uv_len;
    x264_t *videoEncoder = 0;
    x264_picture_t *pic_in = 0;
    pthread_mutex_t mutex;
    VideoCallback videoCallback;

};


#endif //RTMP_PUSHER_VIDEOCHANNEL_H
