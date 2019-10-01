package com.albert.rtmp_pusher;

import android.app.Activity;
import android.view.SurfaceHolder;

public class VideoChannel {
    private CameraHelper cameraHelper;
    private int bitrate;

    public VideoChannel(Activity activity, int cameraId, int width, int height, int bitrate) {
        this.bitrate = bitrate;
        cameraHelper = new CameraHelper(activity, cameraId, width, height);
    }

    public void setPreviewDisplay(SurfaceHolder holder) {
        cameraHelper.setPreviewDisplay(holder);
    }

    public void switchCamera() {
        cameraHelper.switchCamera();
    }

    public void startLive() {

    }

    public void stopLive() {

    }
}
