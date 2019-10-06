package com.albert.rtmp_pusher;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class AudioChannel {

    //AudioRecord
    private NEPusher mPusher;
    private boolean isLive;

    private int channels = 2;
    private AudioRecord mRecord;
    private ExecutorService executorService;
    private int inputSamples;

    public AudioChannel(NEPusher pusher) {
        executorService = Executors.newSingleThreadExecutor();
        this.mPusher = pusher;
        int channelConfig;
        if (channels == 2) {
            channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        } else {
            channelConfig = AudioFormat.CHANNEL_IN_MONO;

        }
        mPusher.native_initAudioEncoder(44100, channels);
        inputSamples = mPusher.getInputSamples() * 2;
        int minBufferSize = AudioRecord.getMinBufferSize(
                44100, channelConfig, AudioFormat.ENCODING_PCM_16BIT) * 2;

        mRecord = new AudioRecord(MediaRecorder.AudioSource.MIC,
                44100,
                channelConfig,
                AudioFormat.ENCODING_PCM_16BIT,
                Math.max(minBufferSize, inputSamples));//TODO 需要校验缓存区大小
    }

    public void startLive() {
        isLive = true;
        executorService.submit(new AudioTask());

    }

    public void stopLive() {
        isLive = false;
    }

    public void release() {

    }


    private class AudioTask implements Runnable {
        @Override
        public void run() {
            mRecord.startRecording();
            byte[] bytes = new byte[inputSamples];
            while (isLive) {
                //每次读多少数据根据编码器定
                int len = mRecord.read(bytes, 0, bytes.length);
                if (len > 0) {
                    mPusher.native_pushAudio(bytes);
                }
            }

            mRecord.stop();
        }
    }
}

