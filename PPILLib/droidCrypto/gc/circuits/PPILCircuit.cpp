//
// Created by chris on 13.11.20.
//

#include "PPILCircuit.h"
#include <droidCrypto/ChannelWrapper.h>
#include <droidCrypto/utils/Log.h>
#include <droidCrypto/BitVector.h>
#include <droidCrypto/gc/HalfGate.h>
#include <droidCrypto/gc/WireLabel.h>
#include <cassert>
#include <cmath>
#include <droidCrypto/as/DeltaShare.h>
#include "Config.h"


namespace droidCrypto {

    //Circuit for RSS == 1
    std::vector<std::vector<WireLabel>> PPILCircuit::computeFunction(const std::vector<std::vector<WireLabel>> &database, const std::vector<WireLabel> &fingerprint, GCEnv &env) {
        std::vector<std::vector<WireLabel>> output;
        assert(M == database.size());
        assert(N == fingerprint.size());
        //XOR Data with Client input
        for(int i = 0; i < M; i++){
            std::vector<WireLabel> tmp;
            assert(N == database[i].size());
            for(int j = 0; j < N; j++){
                tmp.push_back(env.XOR(fingerprint[j], database[i][j]));
            }
            output.push_back(tmp);
        }
        assert(output.size() == M);
        //compute hammingweight
        for(int i = 0; i < M; i++){
            output[i] = hammingweight(output[i], env);
        }
        DISTANCE_BIT_SIZE = ceil(log2(N));
        //KNN-Algorithm
        output = knnAlgo(K, M, output, env);
        return output;
    }

    //Circuit for RSS > 1
    std::vector<std::vector<WireLabel>> PPILCircuit::computeFunction(const std::vector<std::vector<WireLabel>> &inputServer, const std::vector<std::vector<WireLabel>> &inputClient, GCEnv &env) {
        //return inputClient;
        std::vector<std::vector<WireLabel>> output;
        assert(inputServer.size() == inputClient.size());
        assert(M == inputServer.size());
        WireLabel carry;
        std::vector<std::vector<WireLabel>> distances;
        //A2Y
        for (int i = 0; i < M; ++i) {
            std::vector<WireLabel> distance = subGT(const_cast<std::vector<WireLabel> &>(inputServer[i]),const_cast<std::vector<WireLabel> &>(inputClient[i]), env);
            distances.push_back(distance);
        }
        //KNN
        output = knnAlgo(K, M, distances, env);
        return output;
    }

    std::vector<WireLabel> PPILCircuit::computeFunction(const std::vector<WireLabel>& inputA, const std::vector<WireLabel>& inputB, GCEnv& env) {
        std::vector<WireLabel> outputs;
        WireLabel s;
        /*for(int i = 0; i < inputA.size(); i++){
            s = env.XOR(inputA[i], inputB[i]);
            outputs.insert(outputs.begin(), s);
        }*/
        //outputs = additionGT(inputA, inputB, s.getZEROLabel(),env);
        //outputs = onebitAdditionGT(inputA[0], inputB[0], env.NOT(s.getZEROLabel()), env);
        //outputs = inputB;

        outputs = hammingweight(inputB, env);
        /*std::vector<WireLabel> inA = inputA;
        std::vector<WireLabel> inB = inputB;
        WireLabel sel = env.NOT(s.getZEROLabel());*/
        //conditionalSwap(inA, inB, sel, env);
        //outputs = inA;
        //outputs.push_back(GreaterThanGT(inputA, inputB, env));
        //std::vector<std::vector<WireLabel>> ids = createIDVector(inputA.size(), env);
        //outputs = ids[0];
        //std::vector<std::vector<WireLabel>> cmp {inputA, inputB};
        //cmp = knnAlgo(1, cmp.size(), cmp, env);
        //outputs = cmp[0];
        //outputs = ConsGT(3, 4, env);
        return outputs;
    }

    std::vector<WireLabel>
    PPILCircuit::onebitAdditionGT(const WireLabel &inputA, const WireLabel &inputB, const WireLabel &inCarry, GCEnv &env) {
        WireLabel s = env.XOR(env.XOR(inputA, inputB), inCarry);
        WireLabel carry = env.XOR(env.AND(env.XOR(inputA, inputB), env.XOR(inputA, inCarry)),inputA);
        std::vector<WireLabel> outputs;
        outputs.push_back(s);
        outputs.push_back(carry);
        return outputs;
    }


    std::vector<WireLabel>
    PPILCircuit::additionGT(std::vector<WireLabel>& a, std::vector<WireLabel>& b, const WireLabel& inCarry, GCEnv& env) {
        std::vector<WireLabel> outputs;
        WireLabel carry = inCarry;
        if(a.size() != b.size()){
            equalizeSize(a,b);
        }
        for(int i = 0; i < a.size(); i++){
            WireLabel s = env.XOR(env.XOR(a[i], b[i]), carry);
            carry = env.XOR(env.AND(env.XOR(a[i], b[i]), env.XOR(a[i], carry)),a[i]);
            outputs.push_back(s);
        }
        outputs.push_back(carry);
        return outputs;
    }

    std::vector<WireLabel>
    PPILCircuit::subGT(std::vector<WireLabel>& a, std::vector<WireLabel>& b, GCEnv& env) {
        std::vector<WireLabel> outputs;
        WireLabel c;
        WireLabel carry = env.NOT(c.getZEROLabel());
        std::vector<WireLabel> complementB;
        for (int i = 0; i < b.size(); ++i) {
            WireLabel tmp = env.NOT(b[i]);
            complementB.push_back(tmp);
        }
        for(int i = 0; i < a.size(); i++){
            WireLabel s = env.XOR(env.XOR(a[i], complementB[i]), carry);
            carry = env.XOR(env.AND(env.XOR(a[i], complementB[i]), env.XOR(a[i], carry)),a[i]);
            outputs.push_back(s);
        }
        return outputs;
    }

    std::vector<WireLabel> PPILCircuit::hammingweight(const std::vector<WireLabel> wl, GCEnv& env){
        const size_t len = wl.size();
        return hammingweightRec(wl, len, env);
    }

    std::vector<WireLabel> PPILCircuit::hammingweightRec(const std::vector<WireLabel>& wl, const size_t len, GCEnv& env) {
        WireLabel i;
        std::vector<WireLabel> out;
        switch (len) {
            case 0:
                out.push_back(i.getZEROLabel());
                break;
            case 1:
                out.push_back(wl[0]);
                break;
            case 2:
                out = onebitAdditionGT(wl[1], wl[0],i.getZEROLabel(),env);
                break;
            case 3:
                out = onebitAdditionGT(wl[2], wl[1], wl[0],env);
                break;
            default:
                //compute sizes
                size_t u_len = pow(2, (int) log2(len)) - 1;
                size_t v_len = len - u_len - 1;
                std::vector<WireLabel> u_split;
                std::vector<WireLabel> v_split;
                i = wl[0];
                for(int j = 1; j < len; j++){
                    if(j <= v_len){
                        v_split.push_back(wl[j]);
                    } else{
                        u_split.push_back(wl[j]);
                    }
                }
                //recursive call
                std::vector<WireLabel> u = hammingweightRec(u_split, u_len, env);
                std::vector<WireLabel> v = hammingweightRec(v_split, v_len, env);
                //addition
                out = additionGT(u,v,i,env);
        }
        return out;
    }

    std::vector<std::vector<WireLabel>> PPILCircuit::knnAlgo(const int k, const int m, const std::vector<std::vector<WireLabel>>& distances, GCEnv& env){
        std::vector<std::vector<WireLabel>> minDist;
        std::vector<std::vector<WireLabel>> minID;
        size_t idbitlen = ceil(log2(distances.size()));
        int maxValue = pow(2,DISTANCE_BIT_SIZE) -1;
        int maxValueLength = DISTANCE_BIT_SIZE;
        for(int i = 0; i < k+1; i++){
            minDist.push_back(ConsGT(maxValue,maxValueLength, env));
            minID.push_back(ConsGT(0, idbitlen, env));
        }
        for(int i = 0; i < m; i++){
            minDist[k] = distances[i];
            minID[k] = ConsGT(i, idbitlen, env);
            for(int j = k; j > 0; j--){
                WireLabel gt = GreaterThanGT(minDist[j-1], minDist[j], env);
                conditionalSwap(minDist[j], minDist[j-1], gt, env);
                conditionalSwap(minID[j], minID[j-1], gt, env);
            }
        }
        std::vector<std::vector<WireLabel>> result;
        for (int i = 0; i < k; i++) {
            result.push_back(minID[i]);
        }
        return result;
    }

    void PPILCircuit::conditionalSwap(std::vector<WireLabel>& inputA, std::vector<WireLabel>& inputB, WireLabel& select, GCEnv& env){
        WireLabel ab;
        WireLabel z;
        if(inputA.size() != inputB.size()){
            equalizeSize(inputA, inputB);
        }
        for(int i = 0; i < inputA.size(); i++){
            ab = env.XOR(inputA[i], inputB[i]);
            z = env.AND(ab, select);
            inputA[i] = env.XOR(inputA[i], z);
            inputB[i] = env.XOR(inputB[i], z);
        }
    }

    WireLabel PPILCircuit::GreaterThanGT(std::vector<WireLabel>& inputA, std::vector<WireLabel> inputB, GCEnv& env){
        WireLabel ci, ac, bc, acNbc;
        if(inputA.size() != inputB.size()){
           equalizeSize(inputA, inputB);
        }
        ci = ci.getZEROLabel();
        for(int i = 0; i < inputA.size(); i++){
            ac = env.XOR(inputA[i], ci);
            bc = env.XOR(inputB[i], ci);
            acNbc = env.AND(ac, bc);
            ci = env.XOR(acNbc, inputA[i]);
        }
        return ci;
    }

    std::vector<WireLabel> PPILCircuit::ConsGT(int value, size_t length, GCEnv& env){
        std::vector<WireLabel> id;
        //garble value
        WireLabel wl;
        int n = value;
        while(n > 0){
            int bit = n % 2;
            if(bit == 1){
                id.push_back(env.NOT(wl.getZEROLabel()));
            } else {
                id.push_back(wl.getZEROLabel());
            }
            n = n/2;
        }
        //add padding zeros
        for(int l = id.size(); l < length; l++){
            id.push_back(wl.getZEROLabel());
        }

        return id;
    }

    void PPILCircuit::equalizeSize(std::vector<WireLabel>& inputA, std::vector<WireLabel>& inputB){
        WireLabel wl;
        if(inputA.size() > inputB.size()){
            for(int i = inputB.size(); i < inputA.size(); i++){
                inputB.push_back(wl.getZEROLabel());
            }
        } else {
            for(int i = inputA.size(); i < inputB.size(); i++){
                inputA.push_back(wl.getZEROLabel());
            }
        }
    }

/*------SIMD-----------------------------------------------------------------------------------------------------------------------*/

    std::vector<SIMDWireLabel>
    SIMDPPILCircuit::computeFunction(const std::vector<SIMDWireLabel> &inputA,
                                     const std::vector<SIMDWireLabel> &inputB, SIMDGCEnv &env) {

        assert(inputA.size() == BIT_NUMBER);
        assert(inputB.size() == BIT_NUMBER);

        std::vector<SIMDWireLabel> outputs;
        SIMDWireLabel s;
        for(int i = 0; i < inputA.size(); i++){
            s = env.XOR(inputA[i], inputB[i]);
            outputs.push_back(s);
        }
        return outputs;
    }


}


