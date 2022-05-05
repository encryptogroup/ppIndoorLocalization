//
// Created by chris on 13.04.21.
//

#include "droidCrypto/ChannelWrapper.h"
#include "droidCrypto/utils/Benchmark.h"

#ifndef DROIDCRYPTO_COMMUNICATIONHANDLER_H
#define DROIDCRYPTO_COMMUNICATIONHANDLER_H

namespace droidCrypto {
    class CommunicationHandler {
    public:
        CommunicationHandler(droidCrypto::CSocketChannel &chan, bool server): channel(chan){
            prevBytesSent = 0;
            prevBytesRecv = 0;
            isServer = server;
            for (int i = 0; i < logInformation.size(); ++i) {
                logInformation[i] = 0;
            }
        };
        void sendValue(int value, int numberOfBlocks);
        int recvValue(int numberOfBlocks);

        void sendValues(std::vector<int> values, int numberOfBlocks);
        std::vector<int> recvValues(int numberOfValues, int numberOfBlocks);

        int unpackBlock(block value, int relevantBytes);
        block packBlock(int value);

        droidCrypto::CSocketChannel channel;

        void saveBenchmark(Benchmark benchmark, int benchmarkValue);
        void storeLog(int N, int M, int L, int k);
        void logCommunicationSize(Benchmark sending, Benchmark receiving);
        int prevBytesSent;
        int prevBytesRecv;
        bool isServer;
        const int NOT_IN_CSV = 50;
        const Benchmark BASE_OT_AS_TIME = Benchmark( "BASE_OT_AS_TIME", 1);
        const Benchmark BASE_OT_AS_RECV = Benchmark( "BASE_OT_AS_DOWN", 2);
        const Benchmark BASE_OT_AS_SENT = Benchmark( "BASE_OT_AS_UP", 3);
        const Benchmark MT_TIME = Benchmark( "MT_TIME", 10);
        const Benchmark MT_SENT = Benchmark( "MT_UP", 11);
        const Benchmark MT_RECV = Benchmark( "MT_DOWN", 12);
        const Benchmark OFFLINE_AS_TIME = Benchmark( "OFFLINE_AS_TIME", NOT_IN_CSV);
        const Benchmark OFFLINE_AS_SENT = Benchmark( "OFFLINE_AS_UP", NOT_IN_CSV);
        const Benchmark OFFLINE_AS_RECV = Benchmark( "OFFLINE_AS_DOWN", NOT_IN_CSV);
        const Benchmark ONLINE_AS_TIME = Benchmark( "ONLINE_AS_TIME", NOT_IN_CSV);
        const Benchmark ONLINE_AS_SENT = Benchmark( "ONLINE_AS_UP", NOT_IN_CSV);
        const Benchmark ONLINE_AS_RECV = Benchmark( "ONLINE_AS_DOWN", NOT_IN_CSV);
        const Benchmark BASE_OT_GC_TIME = Benchmark( "BASE_OT_GC_TIME", 4);
        const Benchmark BASE_OT_GC_SENT = Benchmark( "BASE_OT_GC_UP", 5);
        const Benchmark BASE_OT_GC_RECV = Benchmark( "BASE_OT_GC_DOWN", 6);
        const Benchmark GC_OT_TIME = Benchmark( "GC_OT_TIME", 13);
        const Benchmark GC_OT_SENT = Benchmark( "GC_OT_UP", 14);
        const Benchmark GC_OT_RECV = Benchmark( "GC_OT_DOWN", 15);
        const Benchmark GC_EVALUATION_TIME = Benchmark( "GC_EVALUATION_TIME", 28);
        const Benchmark GC_GARBLING_TIME = Benchmark( "GC_GARBLING_TIME", 16);
        const Benchmark GC_GARBLING_SENT = Benchmark( "GC_GARBLING_UP", 17);
        const Benchmark GC_GARBLING_RECV = Benchmark( "GC_GARBLING_DOWN", 18);
        const Benchmark GC_TOTAL_TIME = Benchmark( "GC_TOTAL_TIME", NOT_IN_CSV);
        const Benchmark GC_TOTAL_SENT = Benchmark( "GC_TOTAL_UP", NOT_IN_CSV);
        const Benchmark GC_TOTAL_RECV = Benchmark( "GC_TOTAL_DOWN", NOT_IN_CSV);
        const Benchmark GC_SENDING_INPUTS_TIME = Benchmark( "GC_SERVER_INPUTS_TIME", 25);
        const Benchmark GC_SENDING_INPUTS_SENT = Benchmark( "GC_SERVER_INPUTS_UP", 26);
        const Benchmark GC_SENDING_INPUTS_RECV = Benchmark( "GC_SERVER_INPUTS_DOWN", 27);
        const Benchmark GC_NUM_XOR = Benchmark( "GC_NUM_XOR", 31);
        const Benchmark GC_NUM_AND = Benchmark( "GC_NUM_AND", 32);
        const Benchmark GC_OUTPUT_TIME = Benchmark( "GC_OUTPUT_TIME", NOT_IN_CSV);
        const Benchmark GC_OUTPUT_SENT = Benchmark( "GC_EVALUATION_UP", 29);
        const Benchmark GC_OUTPUT_RECV = Benchmark( "GC_EVALUATION_DOWN", 30);
        const Benchmark TOTAL_TIME = Benchmark( "TOTAL_TIME", NOT_IN_CSV);
        const Benchmark AS_CLIENT_SHARING_TIME = Benchmark("CLIENT_INPUT_SHARING_TIME", 19);
        const Benchmark AS_CLIENT_SHARING_SENT = Benchmark("CLIENT_INPUT_SHARING_UP", 20);
        const Benchmark AS_CLIENT_SHARING_RECV = Benchmark("CLIENT_INPUT_SHARING_DOWN", 21);
        const Benchmark AS_DISTANCE_COMPUTATION_TIME = Benchmark("DISTANCE_COMPUTATION_TIME", 22);
        const Benchmark AS_DISTANCE_COMPUTATION_SENT = Benchmark("DISTANCE_COMPUTATION_UP", 23);
        const Benchmark AS_DISTANCE_COMPUTATION_RECV = Benchmark("DISTANCE_COMPUTATION_DOWN", 24);
        const Benchmark AS_SERVER_SHARING_TIME = Benchmark("SERVER_INPUT_SHARING_TIME", 7);
        const Benchmark AS_SERVER_SHARING_SENT = Benchmark("SERVER_INPUT_SHARING_UP", 8);
        const Benchmark AS_SERVER_SHARING_RECV = Benchmark("SERVER_INPUT_SHARING_DOWN", 9);
        const Benchmark AS_SERVER_INPUT_PREP_TIME = Benchmark("AS_SERVER_INPUT_PREP_TIME", NOT_IN_CSV);
        const Benchmark AS_SERVER_INPUT_PREP_SENT = Benchmark("AS_SERVER_INPUT_PREP_UP", NOT_IN_CSV);
        const Benchmark AS_SERVER_INPUT_PREP_RECV = Benchmark("AS_SERVER_INPUT_PREP_DOWN", NOT_IN_CSV);


    private:
        std::vector<uint8_t> packBlocks(int value, int numberOfBlocks);

        int unpackBlocks(uint8_t *blockPointer, int numberOfBlocks, int blockCnt);

        int unpackBlocks(std::vector<block> blocks, int numberOfBlocks);

        int unpackBlocks(std::vector<block> blocks, int numberOfBlocks, int numberOfValues);

        std::array<int, 33> logInformation;
        std::array<std::string, 33> logTags;

        int logArrayCnt = 0;


    };
}


#endif //DROIDCRYPTO_COMMUNICATIONHANDLER_H
