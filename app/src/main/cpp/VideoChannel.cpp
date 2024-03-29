//
// Created by lixiaoxu on 2019-10-01.
//




#include "VideoChannel.h"


VideoChannel::VideoChannel() {
    pthread_mutex_init(&mutex, 0);
}

VideoChannel::~VideoChannel() {
    pthread_mutex_destroy(&mutex);
    if (videoEncoder) {
        x264_encoder_close(videoEncoder);
        videoEncoder = 0;
    }
    if (pic_in) {
        x264_picture_clean(pic_in);
        DELETE(pic_in)
    }
}

/**
 *
 * 初始化x264编码器
 *
 * @param w
 * @param h
 * @param fps
 * @param bitrate
 */
void VideoChannel::initVideoEncoder(int w, int h, int fps, int bitrate) {

    //编码时候可能会发生宽高改变，会调用该方法，会导致编码器重复初始化
    //正在使用编码器，产生冲突
    //为了避免冲突，加入互斥锁
    pthread_mutex_lock(&mutex);
//    mWidth = w;
//    mHeight = h;
//    mFps = fps;
//    mBitrate = bitrate;

    y_len = w * h;
    uv_len = y_len / 4;

    //TODO 先对videoEncoder和pic_in判断
    if (videoEncoder) {
        x264_encoder_close(videoEncoder);
        videoEncoder = 0;
    }
    if (pic_in) {
        x264_picture_clean(pic_in);
        DELETE(pic_in);
    }

    //初始化x264编码器
    x264_param_t param;
    //ultrafast 最快
    //zerolatency 0延时
    x264_param_default_preset(&param, "ultrafast", "zerolatency");
    //编码规则 base_line 3.2
    //https://wikipedia.tw.wjbk.site/wiki/H.264
    param.i_level_idc = 32;
    //输入格式为YUV420P
    param.i_csp = X264_CSP_I420;

    param.i_width = w;
    param.i_height = h;
    //没有B帧
    param.i_bframe = 0;

    //码率控制方式，CQP恒定质量，CRF恒定码率，ABR平均码率
    param.rc.i_rc_method = X264_RC_CRF;
//    param.rc.i_rc_method = X264_RC_ABR;

    //码率（比特率）单位Kb/s
    param.rc.i_bitrate = bitrate / 1000;
    //瞬时最大码率
    param.rc.i_vbv_max_bitrate = bitrate / 1000 * 1.2;
    //设置了i_vbv_max_bitrate就必须设置buffer大小，码率控制大小，单位Kb/s
    param.rc.i_vbv_buffer_size = bitrate / 1000;

    //码率控制不是通过timebase和timestamp，而是通过fps
    param.b_vfr_input = 0;
    //帧率分子
    param.i_fps_num = fps;
    //帧率分母
    param.i_fps_den = 1;
    param.i_timebase_den = param.i_fps_num;
    param.i_timebase_num = param.i_fps_den;

    //帧距离（关键帧）2s一个关键帧
    param.i_keyint_max = fps * 2;
    //是否赋值sps和pps放在每个关键帧的前面，该参数设置是让每个关键帧（I帧）都附带sps/pps
    param.b_repeat_headers = 1;
    //并行编码线程数
    param.i_threads = 1;
    //profile级别，baseline级别
    x264_param_apply_profile(&param, "baseline");
    //输入图像初始化
    pic_in = new x264_picture_t;
    x264_picture_alloc(pic_in, param.i_csp, param.i_width, param.i_height);

    //打开编码器
    videoEncoder = x264_encoder_open(&param);
    if (videoEncoder) {
        LOGI("x264编码打开成功");
    } else {
        LOGE("x264编码打开失败");
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * 编码图像数据
 * @param data
 */
void VideoChannel::encodeData(int8_t *data) {
    pthread_mutex_lock(&mutex);

    //y数据
    memcpy(pic_in->img.plane[0], data, y_len);
    for (int i = 0; i < uv_len; ++i) {
        //u平面数据，拷贝nv21的uv区域奇数位置
        *(pic_in->img.plane[1] + i) = *(data + y_len + i * 2 + 1);
        //v平面数据，拷贝nv21的uv区域偶数位置
        *(pic_in->img.plane[2] + i) = *(data + y_len + i * 2);

    }

    //通过H264编码得到NAL数组
    x264_nal_t *nal = 0;
    int pi_nal;
    x264_picture_t pic_out;

    //进行编码
    int ret = x264_encoder_encode(videoEncoder, &nal, &pi_nal, pic_in, &pic_out);
    if (ret < 0) {
        LOGE("x264编码失败");
        pthread_mutex_unlock(&mutex);
        return;
    }else{
//        LOGI("x264编码成功");
    }
    //sps pps:告诉我们如何编码图像
    int sps_len, pps_len;
    uint8_t sps[100];
    uint8_t pps[100];
    pic_in->i_pts += 1;
    for (int i = 0; i < pi_nal; ++i) {
        if (nal[i].i_type == NAL_SPS) {
            sps_len = nal[i].i_payload - 4;//去掉起始码长度
            memcpy(sps, nal[i].p_payload + 4, sps_len);
        } else if (nal[i].i_type == NAL_PPS) {
            pps_len = nal[i].i_payload - 4;//去掉起始码长度
            memcpy(pps, nal[i].p_payload + 4, pps_len);
            //pps是跟在sps后面，这里达到pps表示前面sps肯定拿到了
            sendSpsPps(sps, pps, sps_len, pps_len);
        } else {
            //帧类型
            sendFrame(nal[i].i_type, nal[i].i_payload, nal[i].p_payload);
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * 发送sps pps包
 *
 * @param sps
 * @param pps
 * @param sps_len
 * @param pps_len
 */
void VideoChannel::sendSpsPps(uint8_t *sps, uint8_t *pps, int sps_len, int pps_len) {
    RTMPPacket *packet = new RTMPPacket;
    int body_size = 5 + 8 + sps_len + 3 + pps_len;//参考图表
    RTMPPacket_Alloc(packet, body_size);

    int i = 0;
    packet->m_body[i++] = 0x17;

    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;
    packet->m_body[i++] = 0x00;

    packet->m_body[i++] = 0x01;

    packet->m_body[i++] = sps[1];
    packet->m_body[i++] = sps[2];
    packet->m_body[i++] = sps[3];

    packet->m_body[i++] = 0xFF;
    packet->m_body[i++] = 0xE1;

    packet->m_body[i++] = (sps_len >> 8) & 0xFF;
    packet->m_body[i++] = sps_len & 0xFF;

    memcpy(&packet->m_body[i], sps, sps_len);

    i += sps_len;//拷贝玩sps数据，i移位

    packet->m_body[i++] = 0x01;

    packet->m_body[i++] = (pps_len >> 8) & 0xFF;
    packet->m_body[i++] = pps_len & 0xFF;

    memcpy(&packet->m_body[i], pps, pps_len);

    i += pps_len;//拷贝完pps数据，i移位

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;//包类型
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 10;
    packet->m_nTimeStamp = 0;//sps pps包没有时间戳
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

    //把数据包放入队列
    videoCallback(packet);
}

void VideoChannel::setVideoCallback(VideoChannel::VideoCallback videoCallback) {
    this->videoCallback = videoCallback;
}

/**
 * 发送帧数据包
 *
 * @param type
 * @param payload
 * @param pPayload
 */
void VideoChannel::sendFrame(int type, int payload, uint8_t *pPayload) {
    //去掉起始码
    if (pPayload[2] == 0x00) {
        pPayload += 4;
        payload -= 4;
    } else if (pPayload[2] == 0x01) {
        pPayload += 3;
        payload -= 3;
    }

    RTMPPacket *packet = new RTMPPacket;
    int body_size = 5 + 4 + payload;//参考图表
    RTMPPacket_Alloc(packet, body_size);

    packet->m_body[0] = 0x27;//普通帧
    if (type == NAL_SLICE_IDR) {
        packet->m_body[0] = 0x17;//关键帧
    }
    packet->m_body[1] = 0x01;
    packet->m_body[2] = 0x00;
    packet->m_body[3] = 0x00;
    packet->m_body[4] = 0x00;


    packet->m_body[5] = (payload >> 24) & 0xFF;
    packet->m_body[6] = (payload >> 16) & 0xFF;
    packet->m_body[7] = (payload >> 8) & 0xFF;
    packet->m_body[8] = payload & 0xFF;

    memcpy(&packet->m_body[9], pPayload, payload);

    packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;//包类型
    packet->m_nBodySize = body_size;
    packet->m_nChannel = 10;
    packet->m_nTimeStamp = -1;
    packet->m_hasAbsTimestamp = 0;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

    //把数据包放入队列
    videoCallback(packet);

}

