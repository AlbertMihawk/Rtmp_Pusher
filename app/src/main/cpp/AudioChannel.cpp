//
// Created by lixiaoxu on 2019-10-06.
//



#include "AudioChannel.h"


AudioChannel::AudioChannel() {

}


AudioChannel::~AudioChannel() {
    DELETE(buffer);
    if (audioEncoder) {
        faacEncClose(audioEncoder);
        audioEncoder = 0;
    }
}

void AudioChannel::initAudioEncoder(int sample_rate, int num_channels) {

    mChannels = num_channels;
    /**
     * 第一步
     * 打开编码器
     * inputSamples: 一次最大能接受的样本数（一个样本16bit,2字节）
     * maxOutputBytes: 编码器最大能输出数据的字节数
     *
     */
    audioEncoder = faacEncOpen(sample_rate, num_channels, &inputSamples, &maxOutputBytes);
    if (!audioEncoder) {
        LOGE("Audio 1:打开音频编码器失败");
        return;
    } else {
        LOGI("Audio 1:打开音频编码器");
    }

    /**
     * 第二步
     * 配置编码器参数
     */
    faacEncConfigurationPtr config = faacEncGetCurrentConfiguration(audioEncoder);
    config->mpegVersion = MPEG4;
    config->aacObjectType = LOW;
    config->inputFormat = FAAC_INPUT_16BIT;

    //编码使用RAW数据，需要自己编码,不用ADTS
    config->outputFormat = RAW_STREAM;
    //时域噪声整形（噪声控制,降噪）
    config->useTns = 1;

    config->useLfe = 0;

    int ret = faacEncSetConfiguration(audioEncoder, config);
    if (!ret) {
        LOGE("Audio 2:音频编码器配置参数失败");
    } else {
        LOGI("Audio 2:音频编码器配置参数");
    }

    //输出缓冲区
    buffer = new u_char[maxOutputBytes];


}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

void AudioChannel::encodeData(int8_t *data) {
    int byteLen = faacEncEncode(audioEncoder, reinterpret_cast<int32_t *>(data), inputSamples,
                                buffer,
                                maxOutputBytes);
    if (byteLen > 0) {
        //看图标，拼数据
        RTMPPacket *packet = new RTMPPacket;
        int body_size = 2 + byteLen;
        RTMPPacket_Alloc(packet, body_size);
        packet->m_body[0] = 0xAF;//双声道
        if (mChannels == 1) {
            packet->m_body[0] = 0xAE;//单声道
        }
        //编码出来的音频数据
        packet->m_body[1] = 0x01;

        //音频数据
        memcpy(&packet->m_body[2], buffer, byteLen);

        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nBodySize = body_size;
        packet->m_nChannel = 11;
        packet->m_nTimeStamp = -1;
        packet->m_hasAbsTimestamp = 0;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        audioCallback(packet);
    }
}

RTMPPacket *AudioChannel::getAudioSeqHeader() {

    u_char *ppBuffer;
    u_long len;
    //获取编码器的解码配置信息
    faacEncGetDecoderSpecificInfo(audioEncoder, &ppBuffer, &len);

    //看图标，拼数据
    RTMPPacket *packet = new RTMPPacket;
    int body_size = 2 + len;
    RTMPPacket_Alloc(packet, body_size);
    packet->m_body[0] = 0xAF;//双声道
    if (mChannels == 1) {
        packet->m_body[0] = 0xAE;//单声道
    }
    //这里是序列头，所以是00
    packet->m_body[1] = 0x00;

    //序列头数据
    memcpy(&packet->m_body[2], ppBuffer, len);

    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 11;
    packet->m_nTimeStamp = -1;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;

    return packet;
}

void AudioChannel::setAudioCallback(AudioChannel::AudioCallback audioCallback) {
    this->audioCallback = audioCallback;

}