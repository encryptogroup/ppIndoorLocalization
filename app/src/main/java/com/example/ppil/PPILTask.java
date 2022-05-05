package com.example.ppil;

import android.os.AsyncTask;
import android.util.Log;

import com.example.ppil.Networking.Channel;

import java.util.Arrays;

public class PPILTask extends AsyncTask<Void, Void, Void> {

    private final int M;
    private final int k;
    private final int protNum;
    private final Area area;
    private final String PPIL_TASK_TAG = PPILTask.class.getSimpleName();

    static {
        System.loadLibrary("droidcrypto");
    }
    public PPILTask(int[] settings){
        area = new Area(settings[1], settings[2]);
        M = settings[0];
        protNum = settings[4];
        k = settings[3];
    }

    @Override
    protected Void doInBackground(Void... voids) {
        int[] fingerprint = area.getRandomRSSFingerprint();//{0,0,4,1,0};
        Log.i(PPIL_TASK_TAG, "input: " + Arrays.toString(fingerprint));
        if (protNum == 0){
            Log.i("", "Here am I");
            String input = Arrays.toString(fingerprint).replaceAll("\\[|\\]|,|\\s", "");
            mainMPPIL(null, input, M, k);
        } else {
            mainEPPIL(null, fingerprint, M, area.getL(), k);
        }
        return null;
    }

    @Override
    protected void onPostExecute(Void aVoid) {
         super.onPostExecute(aVoid);
    }

    private native void mainMPPIL(Channel chan, String input, int DBsize, int k);
    private native void mainEPPIL(Channel chan, int[] input, int DBsize, int rssLength, int k);
}

