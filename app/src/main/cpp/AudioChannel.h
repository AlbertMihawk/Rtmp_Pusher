//
// Created by lixiaoxu on 2019-10-06.
//

#ifndef RTMP_PUSHER_AUDIOCHANNEL_H
#define RTMP_PUSHER_AUDIOCHANNEL_H

#include <faac.h>
#include <sys/types.h>
#include "macro.h"
#include <rtmp.h>
#include <cstring>

class AudioChannel {
    typedef void (*AudioCallback)(RTMPPacket *packet);


public:
    AudioChannel();

    virtual ~AudioChannel();

    void initAudioEncoder(int sample_rate, int num_channels);

    void setAudioCallback(AudioCallback audioCallback);

    int getInputSamples();

    void encodeData(int8_t *data);

private:
    u_long inputSamples;
    u_long maxOutputBytes;
    int mChannels;

    faacEncHandle audioEncoder = 0;

    u_char *buffer;
    AudioCallback audioCallback;

};


#endif //RTMP_PUSHER_AUDIOCHANNEL_H
