package com.albert.rtmp_pusher;

import android.app.Activity;
import android.view.SurfaceHolder;

public class NEPusher {

    static {
        System.loadLibrary("native-lib");
    }

    private final VideoChannel videoChannel;
    private final AudioChannel audioChannel;


    public NEPusher(Activity activity, int cameraId, int width, int height, int fps, int bitrate) {
        native_init();
        videoChannel = new VideoChannel(this, activity, cameraId, width, height, fps, bitrate);
        audioChannel = new AudioChannel(this);

    }


    public void setPreviewDisplay(SurfaceHolder holder) {
        videoChannel.setPreviewDisplay(holder);
    }

    public void switchCamera() {
        videoChannel.switchCamera();
    }

    /**
     * 开始推流
     *
     * @param path rtmp地址
     */
    public void startLive(String path) {
        //native层
        native_start(path);
        videoChannel.startLive();
        audioChannel.startLive();//采集micphone数据
    }


    public void stopLive() {
        videoChannel.stopLive();
        audioChannel.stopLive();
        native_stop();
    }

    public void release() {
        videoChannel.release();
        audioChannel.release();
        native_release();
    }

    public int getInputSamples() {
        return native_getInputSamples();
    }

    private native void native_init();

    private native void native_start(String path);

    private native void native_stop();

    private native void native_release();


    public native void native_pushVideo(byte[] data);

    public native int native_getInputSamples();

    public native void native_initVideoEncoder(int w, int h, int fps, int bitrate);

    public native void native_initAudioEncoder(int sampleRate, int numChannels);

    public native void native_pushAudio(byte[] bytes);
}