//
// Created by chris on 27.03.21.
//
#pragma once

#include <jni.h>
#include <droidCrypto/gc/circuits/Circuit.h>
#include "cmath"

extern "C"
JNIEXPORT jintArray JNICALL Java_com_example_ppil_SetupTask_getParameters(
        JNIEnv *env, jobject /*this*/, jobject channel);

extern "C"
JNIEXPORT void JNICALL Java_com_example_ppil_PPILTask_mainMPPIL(
        JNIEnv *env, jobject /*this*/, jobject channel, jstring input, jint DBsize, jint k);

extern "C"
JNIEXPORT void JNICALL Java_com_example_ppil_PPILTask_mainEPPIL(
        JNIEnv *env, jobject /*this*/, jobject channel, jintArray input, jint DBsize, jint rssLength, jint k);

std::vector<droidCrypto::DeltaShare> createFingerprintShare(std::vector<int> fingerprint, int seed, int l);

std::vector<std::vector<droidCrypto::DeltaShare>> createDatabaseShares(int M, int N, int l, droidCrypto::CSocketChannel& chan);
