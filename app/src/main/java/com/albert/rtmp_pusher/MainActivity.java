package com.albert.rtmp_pusher;

import android.hardware.Camera;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("native-lib");
    }

    private CameraHelper cameraHelper;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

//        // Example of a call to a native method
//        TextView tv = findViewById(R.id.sample_text);
//        tv.setText(stringFromJNI());

        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        cameraHelper = new CameraHelper(this, Camera.CameraInfo.CAMERA_FACING_BACK, 480, 800);
        cameraHelper.setPreviewDisplay(surfaceView.getHolder());
    }

//    public native String stringFromJNI();

    public void switchCamera(View view) {
        cameraHelper.switchCamera();
    }

    public void startLive(View view) {
    }

    public void stopLive(View view) {
    }
}


