//
// Created by chris on 31.03.21.
//

#ifndef DROIDCRYPTO_ARITHMETICSHAREENVIROMENT_H
#define DROIDCRYPTO_ARITHMETICSHAREENVIROMENT_H


#include <droidCrypto/ChannelWrapper.h>
#include <droidCrypto/as/DeltaShare.h>
#include "CommunicationHandler.h"
#include <string>
#include <chrono>
#include <droidCrypto/SecureRandom.h>
#include <droidCrypto/ot/TwoChooseOne/KosDotExtSender.h>
#include <droidCrypto/ot/TwoChooseOne/KosDotExtReceiver.h>
#include <droidCrypto/ot/TwoChooseOne/IknpDotExtSender.h>
#include <droidCrypto/ot/TwoChooseOne/IknpDotExtReceiver.h>
#include "droidCrypto/BitVector.h"

namespace droidCrypto {
    class AEnv {
    public:
        AEnv(CommunicationHandler &comm, bool isServer, int m, int n, int l);

        //Env params
        const uint16_t M;
        const int N;
        const int L;
        const int ringSize;
        std::chrono::duration<double> communicationTime;

        //fields
        CommunicationHandler &communicator;
        bool isServer;
        std::vector<DeltaShare> fingerprint;
        std::vector<std::vector<DeltaShare>> database;
        DeltaShare fingerprintSum = DeltaShare(0,0,0);
        std::vector<DeltaShare> dSquareSum;

        //std::vector<std::vector<DeltaShare>> fingerprintShares;


        std::vector<std::vector<int>> multiplicationTriplets;

        //setup methods
        void setFingerprintShares(std::vector<int> fingerprintValues);
        void setDatabaseShares(std::vector<std::vector<int>> databaseValues);
        void recvFingerprintShares();
        void recvDatabaseShares();

        //computation methods
        //uint8_t getNextMT();
        //DeltaShare mult(DeltaShare a, DeltaShare b);
        DeltaShare mult(std::vector<DeltaShare> a, std::vector<DeltaShare> b, int mIterator);
        std::vector<DeltaShare> finalizeMultiplication(std::vector<DeltaShare> multShares);

        std::vector<droidCrypto::BitVector> toYao(std::vector<DeltaShare> distances);


    private:
        //seeds
        uint8_t fingerprintSeed;
        uint8_t databaseSeed;
        uint8_t fingerprintSquaresSeed;
        uint8_t databaseSquaresSeed;

        //further shared values
        std::vector<int> multDeltas;

        droidCrypto::SecureRandom rnd;
        droidCrypto::IknpDotExtSender OTeSender;
        std::vector<std::array<droidCrypto::block, 2>> SenderOTs;
        IknpDotExtReceiver OTeRecv;
        std::vector<block> ReceiverOTs;
        std::vector<block> SenderOTsCor;


        //int counterMT;

        //setup methods
        void createMT();
        std::vector<droidCrypto::DeltaShare> initFingerprintShares(bool squared);
        std::vector<std::vector<droidCrypto::DeltaShare>> initDatabaseShares(bool squared);

        std::string newBitString(int value, size_t size);


        void performSenderBaseOTs(size_t numBaseOTs);

        void performReceiverBaseOTs(size_t numBaseOTs);

        void doOTPhase(const BitVector &choices);

        void generateMultiplicationTriplets();

        void doOTPhase(size_t numOTs, std::vector<int> shares);

        void doOTPhase(const std::string& choices);

        void doOTPhase(const std::string &choices, int numOTs);
    };
}



#endif //DROIDCRYPTO_ARITHMETICSHAREENVIROMENT_H
