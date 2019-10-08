package com.albert.rtmp_pusher;

import android.Manifest;
import android.content.pm.PackageManager;
import android.hardware.Camera;
import android.os.Build;
import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;
import android.view.SurfaceView;
import android.view.View;

public class MainActivity extends AppCompatActivity {

    private NEPusher pusher;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            String[] perms = {Manifest.permission.CAMERA,
                    Manifest.permission.RECORD_AUDIO,
                    Manifest.permission.INTERNET};
            if (checkSelfPermission(
                    perms[0]) == PackageManager.PERMISSION_DENIED || checkSelfPermission(
                    perms[1]) == PackageManager.PERMISSION_DENIED) {
                requestPermissions(perms, 200);
            }
        }

        SurfaceView surfaceView = findViewById(R.id.surfaceView);
        pusher = new NEPusher(this, Camera.CameraInfo.CAMERA_FACING_BACK,
                640, 480, 25, 800000);
        pusher.setPreviewDisplay(surfaceView.getHolder());
    }

//    public native String stringFromJNI();

    public void switchCamera(View view) {
        pusher.switchCamera();
    }

    public void startLive(View view) {
//        pusher.startLive("rtmp://192.168.201.128/myapp/");
        pusher.startLive("rtmp://59.111.90.142/myapp/");
    }

    public void stopLive(View view) {
        pusher.stopLive();
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        pusher.release();
    }
}


