package com.example.weiyuandeng.drawline;

import android.Manifest;
import android.app.Activity;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.os.Bundle;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.WindowManager;
import android.widget.TextView;
import android.widget.SeekBar;
import android.widget.SeekBar.OnSeekBarChangeListener;

import java.io.IOException;

import static android.graphics.Color.blue;
import static android.graphics.Color.green;
import static android.graphics.Color.red;
import static android.graphics.Color.rgb;

public class MainActivity extends Activity implements TextureView.SurfaceTextureListener {
    private Camera mCamera;
    private TextureView mTextureView;
    private SurfaceView mSurfaceView;
    private SurfaceHolder mSurfaceHolder;
    private Bitmap bmp = Bitmap.createBitmap(640, 480, Bitmap.Config.ARGB_8888);
    private Canvas canvas = new Canvas(bmp);
    private Paint paint1 = new Paint();
    private TextView mTextView;
    static long time_before = 0;

    SeekBar myControl;
    SeekBar myControl2;

    int threBri = 0; 
    int threGrey = 0;
    float f = 0;
    int m = 0;
    int n = 0;

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON); 

        myControl = (SeekBar) findViewById(R.id.seek1);
        myControl2 = (SeekBar) findViewById(R.id.seek2);
        mTextView = (TextView) findViewById(R.id.cameraStatus);

        setMyControlListener();
        setMyControl2Listener();

        ActivityCompat.requestPermissions(MainActivity.this, new String[]{Manifest.permission.CAMERA}, 1);
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA) == PackageManager.PERMISSION_GRANTED) {
            mSurfaceView = (SurfaceView) findViewById(R.id.surfaceview);
            mSurfaceHolder = mSurfaceView.getHolder();

            mTextureView = (TextureView) findViewById(R.id.textureview);
            mTextureView.setSurfaceTextureListener(this);

            paint1.setColor(0xffff0000); 
            paint1.setTextSize(24);

            mTextView.setText("started camera");
        } else {
            mTextView.setText("no camera permissions");
        }

    }

    private void setMyControlListener() {
        myControl.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                threBri = progress; 
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }

    private void setMyControl2Listener() {
        myControl2.setOnSeekBarChangeListener(new OnSeekBarChangeListener() {

            int progressChanged = 0;

            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                progressChanged = progress;
                threGrey = progress;
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });
    }


    public void onSurfaceTextureAvailable(SurfaceTexture surface, int width, int height) {
        mCamera = Camera.open();
        Camera.Parameters parameters = mCamera.getParameters();
        parameters.setPreviewSize(640, 480);
        parameters.setFocusMode(Camera.Parameters.FOCUS_MODE_INFINITY); 
        parameters.setAutoExposureLock(true); 
        mCamera.setParameters(parameters);
        mCamera.setDisplayOrientation(90);

        try {
            mCamera.setPreviewTexture(surface);
            mCamera.startPreview();
        } catch (IOException ioe) {
            // Something bad happened
        }
    }

    public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width, int height) {
        // Ignored, Camera does all the work for us
    }

    public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
        mCamera.stopPreview();
        mCamera.release();
        return true;
    }

    public void onSurfaceTextureUpdated(SurfaceTexture surface) {

        mTextureView.getBitmap(bmp);

        final Canvas c = mSurfaceHolder.lockCanvas();
        if (c != null) {
            int[] pixels = new int[bmp.getWidth()];
            for (int startY = 50; startY < bmp.getHeight(); startY=startY+5) {
                bmp.getPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1);


                for (int i = 0; i < bmp.getWidth(); i++) {
                    if (((green(pixels[i]) - red(pixels[i])) > -threGrey)&&((green(pixels[i]) - red(pixels[i])) < threGrey)&&(green(pixels[i])  > threBri)) {
                        pixels[i] = rgb(1, 1, 1); 

                        m = m + green(pixels[i])+red(pixels[i])+blue(pixels[i]);
                        n = n + (green(pixels[i])+red(pixels[i])+blue(pixels[i]))*i;
                    }
                }

                if(m>5){
                     f = n / m;
                    canvas.drawCircle(f, startY,3,paint1);
                }
                else{
                    f = 0;
                }
                bmp.setPixels(pixels, 0, bmp.getWidth(), 0, startY, bmp.getWidth(), 1);
            }
        }

        int pos = 50;
        canvas.drawCircle(pos, 240, 5, paint1);

        canvas.drawText("Brightness = " + threBri +" Grey = " + threGrey, 10, 200, paint1);
        c.drawBitmap(bmp, 0, 0, null);
        mSurfaceHolder.unlockCanvasAndPost(c);

        long now = System.currentTimeMillis();
        long diff = now - time_before;
        mTextView.setText("FPS " + 1000 / diff);
        time_before = now;
    }
}
