package com.example.ppil;

public class Area {

    private int N;
    private int range;//percentage of the fake RSS measurements are non-zero
    private int maxRSS;
    private int L;

    public Area(int numAccessPoints){
        N = numAccessPoints;
        maxRSS = 4;
        range = 15;
    }

    public Area(int numAccessPoints, int l){
        N = numAccessPoints;
        maxRSS = (int) Math.pow(2,l) -1;
        range = 15;
        L = l;
    }

    public int[] getRandomRSSFingerprint(){
        int values[] = new int [N];

        for(int i = 0; i < values.length; i++){
            int rnd = (int) (Math.random()*100);
            if(rnd > range){
                values[i] = 0;
            } else{
                values[i] = ((int) (Math.random() * maxRSS))+1;
            }
        }
        return values;
    }

    public int getL(){
        return L;
    }
}
