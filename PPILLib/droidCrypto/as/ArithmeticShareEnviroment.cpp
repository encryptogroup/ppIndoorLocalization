//
// Created by chris on 31.03.21.
//

#include <droidCrypto/utils/Log.h>
#include <bitset>
#include <Config.h>
#include <droidCrypto/BitVector.h>
#include "ArithmeticShareEnviroment.h"
#include "CommunicationHandler.h"
#include <droidCrypto/Defines.h>
#include <chrono>
#include <droidCrypto/ot/NaorPinkas.h>
#include <droidCrypto/gc/WireLabel.h>
#include <assert.h>
#include "type_traits"



droidCrypto::AEnv::AEnv(CommunicationHandler &comm, bool isServer, int m, int n, int l):
        communicator(comm), M(m), N(n), L(l), ringSize(pow(2, l)), communicationTime(0) {
    //counterMT = 0;
    this->isServer = isServer;
    if(isServer){
        //srand(456);
        databaseSeed = rnd.rand();
        //srand(678);
        databaseSquaresSeed = rnd.rand();
        communicator.sendValue(databaseSeed, 1);
        communicator.sendValue(databaseSquaresSeed, 1);
        fingerprintSeed = communicator.recvValue(1);
        fingerprintSquaresSeed = communicator.recvValue(1);
    } else{
        //srand(532);
        fingerprintSeed = rnd.rand();
        //srand(111);
        fingerprintSquaresSeed = rnd.rand();
        communicator.sendValue(fingerprintSeed, 1);
        communicator.sendValue(fingerprintSquaresSeed, 1);
        databaseSeed = communicator.recvValue(1);
        databaseSquaresSeed = communicator.recvValue(1);
    }

    //BaseOT
    int bytesBaseOTRecvBegin = communicator.channel.getBytesRecv();
    int bytesBaseOTSentBegin = communicator.channel.getBytesSent();
    auto timebaseOTStart = std::chrono::high_resolution_clock::now();
    if (isServer){
        performReceiverBaseOTs(128);
        performSenderBaseOTs(128);
    } else{
        performSenderBaseOTs(128);
        performReceiverBaseOTs(128);
    }
    auto timebaseOTEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> baseOTTime = timebaseOTEnd - timebaseOTStart;
    droidCrypto::Log::v("MA", "BaseOT time arithmetic sharing: %f seconds", baseOTTime);
    int bytesBaseOTRecvEnd = communicator.channel.getBytesRecv();
    int bytesBaseOTSentEnd = communicator.channel.getBytesSent();
    int bytesbaseOTRecv = bytesBaseOTRecvEnd - bytesBaseOTRecvBegin;
    int bytesBaseOTSent = bytesBaseOTSentEnd - bytesBaseOTSentBegin;
    auto timeBaseOT = std::chrono::duration_cast<std::chrono::milliseconds>(timebaseOTEnd - timebaseOTStart);
    comm.saveBenchmark(comm.BASE_OT_AS_TIME, timeBaseOT.count());
    comm.saveBenchmark(comm.BASE_OT_AS_SENT, bytesBaseOTSent);
    comm.saveBenchmark(comm.BASE_OT_AS_RECV, bytesbaseOTRecv);
    droidCrypto::Log::v("AS", "BaseOT MT Bytes sent: %d, MT Bytes recv: %d", bytesBaseOTSent, bytesbaseOTRecv);

    //Create Multiplication Triplets
    auto timeMTStart = std::chrono::high_resolution_clock::now();
    generateMultiplicationTriplets();
    auto timeMTEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> mTTime = timeMTEnd - timeMTStart;
    droidCrypto::Log::v("MA", "MT time arithmetic sharing: %f seconds", mTTime);
    int bytesMTEndRecv = communicator.channel.getBytesRecv();
    int bytesMTEndSent = communicator.channel.getBytesSent();
    int bytesMTRecv = bytesMTEndRecv - bytesBaseOTRecvEnd;
    int bytesMTSent = bytesMTEndSent - bytesBaseOTSentEnd;
    auto timeMT = std::chrono::duration_cast<std::chrono::milliseconds>(timeMTEnd - timeMTStart);
    comm.saveBenchmark(comm.MT_TIME, timeMT.count());
    comm.saveBenchmark(comm.MT_SENT, bytesMTSent);
    comm.saveBenchmark(comm.MT_RECV, bytesMTRecv);
    droidCrypto::Log::v("AS", "MT Bytes sent: %d, MT Bytes recv: %d", bytesMTSent, bytesMTRecv);
}

void droidCrypto::AEnv::generateMultiplicationTriplets(){
    //pre-processing OT
    //setting OT parameters
    int numOTs = M * N * L;
    std::vector<int> correlationShares;
    //BitVector choices;
    std::string choiceString;
    std::vector<DeltaShare> tmpF;

    for(int i = 0; i < M; i++) {
        std::vector<DeltaShare> tmpD;
        for (int j = 0; j < N; ++j) {
            //create random small Deltas for shares
            int databaseDelta;
            int fingerprintDelta;
            if(isServer){
                databaseDelta = rnd.rand() % ringSize;
                if(i == 0){
                    //TODO: Use secure randomness
                    srand(fingerprintSeed + j);
                    fingerprintDelta = rand() % ringSize;
                    fingerprint.push_back(DeltaShare(0, fingerprintDelta, ringSize));
                    std::string fingerprintChoice = newBitString(fingerprintDelta, L);
                    choiceString.append(fingerprintChoice);
                }
            } else{
                if(i == 0){
                    fingerprintDelta = rnd.rand() % ringSize;
                    fingerprint.push_back(DeltaShare(0, fingerprintDelta, ringSize));
                    std::string fingerprintChoice = newBitString(fingerprintDelta, L);
                    choiceString.append(fingerprintChoice);
                }
                //TODO: Use secure randomness
                srand(databaseSeed + (i * N + j));
                databaseDelta = rand() % ringSize;
            }
            //preparing OT
            correlationShares.push_back(databaseDelta);

            tmpD.push_back(DeltaShare(0, databaseDelta, ringSize));

        }
        database.push_back(tmpD);
    }
    //choices = BitVector(choiceString);

    //assert(choices.size() == numOTs);
    assert(correlationShares.size() == M*N);

    //OT
    if (!isServer){
        doOTPhase(numOTs, correlationShares);
        doOTPhase(choiceString, numOTs);
    } else{
        doOTPhase(choiceString, numOTs);
        doOTPhase(numOTs, correlationShares);
    }

    assert(SenderOTsCor.size() == numOTs);
    assert(ReceiverOTs.size() == numOTs);

    //post-processing OT
    long otIt = 0;
    for (int i = 0; i < M; ++i) {
        std::vector<int> tmpM;
        for (int j = 0; j < N; ++j) {
            long u = 0, v=0, c;
            for (int k = 0; k < L; ++k) {
                int s0 = communicator.unpackBlock(SenderOTsCor[otIt], 2);
                int sC = communicator.unpackBlock(ReceiverOTs[otIt],2);
                s0 = s0 % ringSize;
                sC = sC % ringSize;

                u = positive_modulo(ringSize, u + sC);
                v = positive_modulo(ringSize, v - s0);
                //droidCrypto::Log::v("AS", "k: %d, sC: %d, u: %d, v: %d", k, sC, u, v);
                //droidCrypto::Log::v("AS", " s0: %d", s0);
                otIt++;
            }
            u = positive_modulo(ringSize, u);
            v = positive_modulo(ringSize, v);
            c = positive_modulo(ringSize,(database[i][j].getSmallDeltaShare() * fingerprint[j].getSmallDeltaShare() + u + v));
            tmpM.push_back(c);
        }
        multiplicationTriplets.push_back(tmpM);
    }
}

void droidCrypto::AEnv::setFingerprintShares(std::vector<int> fingerprintValues) {
    std::vector<int> fingerprintCDeltas;
    int squareSum = 0;
    for (int i = 0; i < N; ++i) {
        fingerprint[i].setValue(fingerprintValues[i], fingerprintSeed + i);
        fingerprintCDeltas.push_back(fingerprint[i].getCapitalDelta());
        squareSum = squareSum + pow(fingerprintValues[i],2);
    }
    int temp = rnd.rand() % ringSize;
    fingerprintSum = DeltaShare(0, temp, ringSize);
    fingerprintSum.setValue(squareSum, fingerprintSquaresSeed);
    fingerprintCDeltas.push_back(fingerprintSum.getCapitalDelta());

    communicator.sendValues(fingerprintCDeltas, 2);
}

void droidCrypto::AEnv::setDatabaseShares(std::vector<std::vector<int>> databaseValues) {
    std::vector<int> databaseCDelta;
    //create shares for database values
    for (int i = 0; i < M; ++i) {
        int squareSum = 0;
        for (int j = 0; j < N; ++j) {
            database[i][j].setValue(databaseValues[i][j], databaseSeed + (i * N + j));
            databaseCDelta.push_back(database[i][j].getCapitalDelta());
            squareSum = squareSum + pow(databaseValues[i][j],2);
        }
        int temp = rnd.rand() %ringSize;
        dSquareSum.push_back(DeltaShare(0, temp, ringSize));
        dSquareSum[i].setValue(squareSum, databaseSquaresSeed);
        databaseCDelta.push_back(dSquareSum[i].getCapitalDelta());
    }
    communicator.sendValues(databaseCDelta, 2);
}

void droidCrypto::AEnv::recvFingerprintShares() {
    //std::vector<int> fingerprintCDeltas = communicator.recvValues(N, 2);
    std::vector<int> fingerprintCDeltasMN = communicator.recvValues(N+1, 2);
    //Create Shares for the fingerprint
    for (int i = 0; i < N; ++i) {
        fingerprint[i].setCapitalDelta(fingerprintCDeltasMN[i]);
    }
    //TODO: Use secure randomness
    srand(fingerprintSquaresSeed);
    int temp = rand() % ringSize;
    fingerprintSum = DeltaShare(0, temp, ringSize);
    fingerprintSum.setCapitalDelta(fingerprintCDeltasMN[N]);
}

void droidCrypto::AEnv::recvDatabaseShares() {
    std::vector<int> databaseCDeltas = communicator.recvValues(M*N+M, 2);
    int k = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; ++j) {
            database[i][j].setCapitalDelta(databaseCDeltas[k]);
            k++;
        }
        //TODO: Use secure randomness
        srand(databaseSquaresSeed);
        int temp = rand() % ringSize;
        dSquareSum.push_back(DeltaShare(0, temp, ringSize));
        dSquareSum[i].setCapitalDelta(databaseCDeltas[k]);
        k++;
    }

}

droidCrypto::DeltaShare droidCrypto::AEnv::mult(std::vector<DeltaShare> a, std::vector<DeltaShare> b, int mIterator) {

    //srand(6789 + mIterator);
    int preShare = rnd.rand() % ringSize;
    long sum = 0;
    int i = 0;
    if (isServer){
        i = 1;
    }
    for (int j = 0; j <a.size(); ++j) {
        uint64_t tmp = positive_modulo(ringSize,
                                       ((i * a[j].getCapitalDelta() * b[j].getCapitalDelta()) - (a[j].getCapitalDelta() * b[j].getSmallDeltaShare()) - (b[j].getCapitalDelta() * a[j].getSmallDeltaShare()) + multiplicationTriplets[mIterator][j]));
        sum = (sum + tmp);
        if(mIterator == 0){
            //Log::v("TEST","aD: %d, ad: %d, bD: %d, bd: %d, yab: %d, sum: %d",a[j].getCapitalDelta(), a[j].getSmallDeltaShare(), b[j].getCapitalDelta(), b[j].getSmallDeltaShare(), multiplicationTriplets[mIterator][j], sum);
        }
    }
    sum = positive_modulo(ringSize, sum + preShare);
    multDeltas.push_back(sum);
    //Log::v("TEST", "yD:%d, yd: %d", sum, preShare);
    return DeltaShare(0, preShare, ringSize);
}

std::vector<droidCrypto::DeltaShare> droidCrypto::AEnv::finalizeMultiplication(std::vector<DeltaShare> multShares){
    std::vector<droidCrypto::DeltaShare> shares;
    std::vector<int> otherSums;
    auto timeCommStart = std::chrono::high_resolution_clock::now();
    communicator.sendValues(multDeltas,2);
    otherSums = communicator.recvValues(M, 2);
    auto timeCommEnd = std::chrono::high_resolution_clock::now();
    communicationTime = communicationTime + (timeCommEnd - timeCommStart);

    for (int i = 0; i < M; ++i) {
        int bigDelta = multDeltas[i] + otherSums[i];
        droidCrypto::DeltaShare tmp(bigDelta, multShares[i].getSmallDeltaShare(), ringSize);
        shares.push_back(tmp);
    }
    return shares;
}

std::vector<droidCrypto::BitVector> droidCrypto::AEnv::toYao(std::vector<droidCrypto::DeltaShare> distances){
    int length = log2(ringSize);
    size_t len(length);
    std::vector<droidCrypto::BitVector> yaoInput;
    int tmp;
    for (int j = 0; j < M; ++j) {
        if (isServer){
            tmp = positive_modulo(ringSize, distances[j].getCapitalDelta() - distances[j].getSmallDeltaShare());
        } else{
            tmp = positive_modulo(ringSize, distances[j].getSmallDeltaShare());
        }
        droidCrypto::BitVector bitvector = newBitString(tmp, len);
        yaoInput.push_back(bitvector);
    }
    return yaoInput;
}

std::vector<droidCrypto::DeltaShare> droidCrypto::AEnv::initFingerprintShares(bool squared){
    std::vector<DeltaShare> fingerprintShares;
    //srand(7654);
    int seed = rnd.rand();
    if(isServer){
        if (squared){
            seed = fingerprintSquaresSeed;
        } else{
            seed = fingerprintSeed;
        }
    }
    for (int i = 0; i < N; ++i) {
        droidCrypto::DeltaShare share = droidCrypto::DeltaShare(seed + i, ringSize);
        fingerprintShares.push_back(share);
    }
    return fingerprintShares;
}

std::vector<std::vector<droidCrypto::DeltaShare>> droidCrypto::AEnv::initDatabaseShares(bool squared) {
    //create shares for database values
    std::vector<std::vector<droidCrypto::DeltaShare>> databaseShares;
    //srand(889);
    int seed = rnd.rand();
    if(!isServer){
        if(squared){
            seed = databaseSquaresSeed;
        } else{
            seed = databaseSeed;
        }
    }
    for (int i = 0; i < M; ++i) {
        std::vector<droidCrypto::DeltaShare> shares;
        for (int j = 0; j < N; ++j) {
            droidCrypto::DeltaShare share = droidCrypto::DeltaShare(seed + (i * N + j),ringSize);
            shares.push_back(share);
        }
        databaseShares.push_back(shares);
    }
    return databaseShares;
}

std::string droidCrypto::AEnv::newBitString(int value, size_t size) {
    std::string binaryNum[size];
    for (int i = 0; i < size; ++i) {
        if(value > 0){
            int tmp  = value % 2;
            if (tmp == 1){
                binaryNum[i] = "1";
            } else{
                binaryNum[i] = "0";
            }
            value = value / 2;
        } else {
            binaryNum[i] = "0";
        }
    }
    std::string s;
    for (int i = 0; i < size; i++) {
        s.append(binaryNum[i]);
    }
    //droidCrypto::BitVector f(s);
    return s;
}

void droidCrypto::AEnv::performSenderBaseOTs(size_t numBaseOTs /* = 128 */) {
    block seed = rnd.randBlock();
    //PRNG p = PRNG::getTestPRNG();
    PRNG p(seed);
    std::vector<block> baseOTs;
    BitVector baseChoices(numBaseOTs);
    baseChoices.randomize(p);
    baseOTs.resize(numBaseOTs);
    span<block> baseOTsSpan(baseOTs.data(), baseOTs.size());

#if defined(ENABLE_SIMPLEST_OT)
    VerifiedSimplestOT ot;
#else
    NaorPinkas ot;
#endif
    ot.receive(baseChoices, baseOTsSpan, p, communicator.channel);
    OTeSender.setBaseOts(baseOTsSpan, baseChoices);
}

void droidCrypto::AEnv::performReceiverBaseOTs(size_t numBaseOTs /* = 128 */) {
#if defined(ENABLE_SIMPLEST_OT)
    VerifiedSimplestOT ot;
#else
    NaorPinkas ot;
#endif
    std::vector<std::array<block, 2>> baseOTs;
    baseOTs.resize(numBaseOTs);
    block seed = rnd.randBlock();
    //PRNG p = PRNG::getTestPRNG();
    PRNG p(seed);
    span<std::array<block, 2>> baseOTsSpan(baseOTs.data(), baseOTs.size());
    ot.send(baseOTsSpan, p, communicator.channel);
    OTeRecv.setBaseOts(baseOTsSpan);
}

void droidCrypto::AEnv::doOTPhase(const std::string& choices, int numOTs) {
    ReceiverOTs.clear();
    ReceiverOTs.resize(numOTs);
    block seed = rnd.randBlock();
    //PRNG p = PRNG::getTestPRNG();
    PRNG p(seed);
    OTeRecv.receiveCorrelated(M, N, choices, span<block>(ReceiverOTs.data(), ReceiverOTs.size()), p, communicator.channel);
}

void droidCrypto::AEnv::doOTPhase(size_t numOTs, std::vector<int> shares) {
    SenderOTsCor.clear();
    SenderOTsCor.resize(numOTs);
    block seed = rnd.randBlock();
    //PRNG p = PRNG::getTestPRNG();
    PRNG p(seed);
    OTeSender.sendCorrelated(span<block>(SenderOTsCor.data(), SenderOTsCor.size()), shares, M, N, L, ringSize, p,
                   communicator.channel);
}










