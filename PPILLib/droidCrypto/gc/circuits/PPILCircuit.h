//
// Created by chris on 13.11.20.
//
#pragma once

#include <jni.h>
#include <droidCrypto/gc/circuits/Circuit.h>
#include "cmath"

namespace droidCrypto {


    class GCEnv;

    class SIMDGCEnv;

#define BIT_NUMBER (8)

    class PPILCircuit : public Circuit{
    public:
        PPILCircuit(ChannelWrapper& chan, int numAP, int dbEntries, int k, int l, int inputElementsLength) : Circuit(chan, inputElementsLength, inputElementsLength, log2(dbEntries)+1) {
            N = numAP;
            M = dbEntries;
            K =k;
            DISTANCE_BIT_SIZE = inputElementsLength;
            RSS_BIT_SIZE = l;}

    protected:

        int M;
        int N;
        int K;
        int RSS_BIT_SIZE;
        int DISTANCE_BIT_SIZE;

        std::vector<WireLabel> computeFunction(const std::vector<WireLabel>& inputA, const std::vector<WireLabel>& inputB, GCEnv& env) override;
        std::vector<std::vector<WireLabel>> computeFunction(const std::vector<std::vector<WireLabel>> &inputServer,
                                                            const std::vector<std::vector<WireLabel>> &inputClient,
                                                            GCEnv &env) override;
        std::vector<std::vector<WireLabel>> computeFunction(const std::vector<std::vector<WireLabel>> &database,
                                                            const std::vector<WireLabel> &fingerprint, GCEnv &env) override;
        std::vector<WireLabel> ConsGT(int value, size_t length, GCEnv &env);

        std::vector<WireLabel> additionGT(std::vector<WireLabel>& a, std::vector<WireLabel>& b, const WireLabel& inCarry, GCEnv& env);
        std::vector<WireLabel> onebitAdditionGT(const WireLabel& inputA, const WireLabel& inputB, const WireLabel& inCarry, GCEnv& env);

        WireLabel GreaterThanGT(std::vector<WireLabel> &inputA, std::vector<WireLabel> inputB, GCEnv &env);

        void conditionalSwap(std::vector<WireLabel> &inputA, std::vector<WireLabel> &inputB, WireLabel &select, GCEnv &env);

        std::vector<WireLabel> hammingweightRec(const std::vector<WireLabel>& wl, const size_t len, GCEnv& env);
        std::vector<WireLabel> hammingweight(const std::vector<WireLabel> wl, GCEnv &env);

        std::vector<std::vector<WireLabel>> knnAlgo(const int k, const int m, const std::vector<std::vector<WireLabel>> &distances, GCEnv &env);

        void equalizeSize(std::vector<WireLabel> &inputA, std::vector<WireLabel> &inputB);

        std::vector<WireLabel> subGT(std::vector<WireLabel> &a, std::vector<WireLabel> &b, GCEnv &env);
    };

    class SIMDPPILCircuit : public SIMDCircuit{
    public:
        SIMDPPILCircuit(ChannelWrapper& chan) : SIMDCircuit(chan, BIT_NUMBER, BIT_NUMBER, BIT_NUMBER) {}

    protected:

        std::vector<SIMDWireLabel> computeFunction(const std::vector<SIMDWireLabel>& inputA, const std::vector<SIMDWireLabel>& inputB, SIMDGCEnv& env) override;

    };



}

