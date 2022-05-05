package com.example.ppil;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;

import com.example.ppil.Networking.Channel;

import java.util.concurrent.ExecutionException;

public class MainActivity extends AppCompatActivity {

    private int[] settings;
    private final String MAIN_TAG = MainActivity.class.getSimpleName();

    static {
        System.loadLibrary("droidcrypto");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.i(MAIN_TAG, "Start Application");
        SetupTask task = new SetupTask();
        task.execute();
        try {
            settings = task.get();
        } catch (ExecutionException | InterruptedException e) {
            e.printStackTrace();
        }
        Log.i(MAIN_TAG, "Area settings initialized");
    }

    public void localization(View v) {
        Log.i(MAIN_TAG, "Start evaluation");
        long startTime = System.nanoTime();
        PPILTask ppilTask = new PPILTask(settings);
        ppilTask.execute();
        try {
            ppilTask.get();
            long stopTime = System.nanoTime();
            double duration = (double) (stopTime - startTime) / 1_000_000_000;
            Log.i(MAIN_TAG, "Evaluation Done in " + duration + "s");
        } catch (ExecutionException | InterruptedException e) {
            e.printStackTrace();
        }
    }

    public void localization5(View v) throws InterruptedException {
        Log.i(MAIN_TAG, "Start evaluation");
        for (int i = 0; i < 10; i++){
            long startTime = System.nanoTime();
            PPILTask ppilTask = new PPILTask(settings);
            ppilTask.execute();
            try {
                ppilTask.get();
                long stopTime = System.nanoTime();
                double duration = (double) (stopTime - startTime) / 1_000_000_000;
                Log.i(MAIN_TAG, "Evaluation Done in " + duration + "s");
            } catch (ExecutionException | InterruptedException e) {
                e.printStackTrace();
            }
            Thread.sleep(500);
        }
    }

}