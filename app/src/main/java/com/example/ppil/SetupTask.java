package com.example.ppil;

import android.os.AsyncTask;
import android.util.Log;

import com.example.ppil.Networking.Channel;

import java.util.Arrays;

public class SetupTask extends AsyncTask<Void, Void, int[]> {

    private final String PPIL_TASK_TAG = PPILTask.class.getSimpleName();

    static {
        System.loadLibrary("droidcrypto");
    }
    public SetupTask(){}

    @Override
    protected int[] doInBackground(Void... voids) {
        Log.i(PPIL_TASK_TAG, "Request setting parameters from Server");
        //[M,N,l,k,p]
        int[] settings = getParameters(null);
        Log.d(PPIL_TASK_TAG, "Received settings of the form [M,N,l,k,p]: " + Arrays.toString(settings));
        return settings;
    }

    @Override
    protected void onPostExecute(int[] aVoid) {
        super.onPostExecute(aVoid);
    }

    private native int[] getParameters(Channel chan);

}

