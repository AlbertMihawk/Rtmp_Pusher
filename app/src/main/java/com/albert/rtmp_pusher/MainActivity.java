package com.albert.rtmp_pusher;

import android.hardware.Camera;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;

public class MainActivity extends AppCompatActivity {

//    static {
//        System.loadLibrary("native-lib");
//    }


    private NEPusher nePusher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        nePusher = new NEPusher(this, Camera.CameraInfo.CAMERA_FACING_BACK, 640, 480, 30, 800000);
        nePusher.setPreviewDisplay(surfaceView.getHolder());

    }

//    public native String stringFromJNI();

    public void switchCamera(View view) {
        nePusher.switchCamera();
    }

    public void startLive(View view) {
//        nePusher.startLive("rtmp://192.168.201.128/myapp/");
        nePusher.startLive("rtmp://59.111.90.142/myapp/");
    }

    public void stopLive(View view) {
        nePusher.stopLive();
    }
}


