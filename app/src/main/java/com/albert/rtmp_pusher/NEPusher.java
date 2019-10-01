package com.albert.rtmp_pusher;

import android.app.Activity;
import android.view.SurfaceHolder;

public class NEPusher {

    static {
        System.loadLibrary("native-lib");
    }

    private final VideoChannel videoChannel;
    private final AudioChannel audioChannel;


    public NEPusher(Activity activity, int cameraId, int width, int height, int bitrate) {
        native_init();
        videoChannel = new VideoChannel(activity, cameraId, width, height, bitrate);
        audioChannel = new AudioChannel();

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
        //audioChannel.startLive();
    }


    public void stopLive() {
        native_stop();
        videoChannel.stopLive();
    }


    private native void native_init();

    private native void native_start(String path);

    private native void native_stop();
}