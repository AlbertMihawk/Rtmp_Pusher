package com.albert.rtmp_pusher;

import android.app.Activity;
import android.hardware.Camera;
import android.util.Log;
import android.view.SurfaceHolder;

public class VideoChannel implements Camera.PreviewCallback, CameraHelper.OnChangedSizeListener {
    public static final String TAG = VideoChannel.class.getCanonicalName();
    private CameraHelper cameraHelper;
    private int mBitrate;
    private int mFps;
    private boolean isLive;
    private NEPusher mPusher;
    private int mWidth;
    private int mHeight;

    public VideoChannel(NEPusher pusher, Activity activity, int cameraId, int width, int height, int mFps, int bitrate) {
        this.mWidth = width;
        this.mHeight = height;
        this.mBitrate = bitrate;
        this.mFps = mFps;
        mPusher = pusher;
        cameraHelper = new CameraHelper(activity, cameraId, width, height);
        cameraHelper.setPreviewCallback(this);
        cameraHelper.setOnChangedSizeListener(this);
    }

    public void setPreviewDisplay(SurfaceHolder holder) {
        cameraHelper.setPreviewDisplay(holder);
    }

    public void switchCamera() {
        cameraHelper.switchCamera();
    }

    public void startLive() {
        Log.i(TAG, "startLive: 开始直播");
        mPusher.native_initVideoEncoder(mWidth, mHeight, mFps, mBitrate);
        isLive = true;
    }

    public void stopLive() {
        Log.i(TAG, "startLive: 停止直播");
        isLive = false;
    }

    @Override
    public void onPreviewFrame(byte[] data, Camera camera) {
        if (isLive) {
            //图像数据推送
            mPusher.native_pushVideo(data);
        }
    }

    @Override
    public void onChanged(int w, int h) {
        if (isLive) {
            //视频编码器的初始化有关 宽，高，mFps，bitrate
            mPusher.native_initVideoEncoder(w, h, mFps, mBitrate);
        }

    }

    public void release() {
        cameraHelper.stopPreview();
    }
}
