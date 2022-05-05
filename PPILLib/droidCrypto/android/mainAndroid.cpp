//
// Created by chris on 27.03.21.
//

#include <droidCrypto/utils/Log.h>
#include <droidCrypto/as/DeltaShare.h>
#include <droidCrypto/ChannelWrapper.h>
#include <droidCrypto/BitVector.h>
#include <droidCrypto/gc/circuits/PPILCircuit.h>
#include "mainAndroid.h"
#include "Config.h"
#include "droidCrypto/as/EuclideanDistancesComputations.h"

uint16_t reinterpretParams(std::vector<droidCrypto::block> paramBlock){
    uint16_t param = 0;
    std::vector<uint8_t> byteBlocks;
    for (int i = 0; i < paramBlock.size(); ++i) {
        uint8_t *x = (uint8_t *) &paramBlock[i];
        byteBlocks.push_back(*x);
    }
    param = (byteBlocks[0] << 8) | byteBlocks[1];
    return param;
}

jintArray Java_com_example_ppil_SetupTask_getParameters(
        JNIEnv *env, jobject /*this*/, jobject channel) {
    //[M,N,l,k,protnum]
    jintArray result = env->NewIntArray(5);
    droidCrypto::CSocketChannel chan(ROUTE_IP, ROUTE_PORT, false);
    std::vector<droidCrypto::block> recM(2);
    std::vector<droidCrypto::block> recN(2);
    droidCrypto::block recL;
    droidCrypto::block recK;
    droidCrypto::block recP;
    chan.recv(recM);
    chan.recv(recN);
    chan.recv(recL);
    chan.recv(recK);
    chan.recv(recP);

    uint16_t M = reinterpretParams(recM);
    uint16_t  N = reinterpretParams(recN);
    uint8_t *l = (uint8_t *) &recL;
    uint8_t *k = (uint8_t *) &recK;
    uint8_t *p = (uint8_t *) &recP;
    jint values[5] = {M,N,*l,*k,*p};
    env->SetIntArrayRegion(result, 0, 5, values);
    return result;
}

void Java_com_example_ppil_PPILTask_mainMPPIL(
        JNIEnv *env, jobject /*this*/, jobject channel, jstring input, jint DBsize, jint k) {
    droidCrypto::Log::v("MA", "Connecting to Server");
    auto totalTimeStart = std::chrono::high_resolution_clock::now();
    droidCrypto::CSocketChannel chan(ROUTE_IP, ROUTE_PORT, false);
    const char *inputC = env->GetStringUTFChars(input, (jboolean*) nullptr);
    droidCrypto::BitVector inVector(inputC);
    droidCrypto::PPILCircuit circ(chan, inVector.size(), DBsize, k, 1, inVector.size());
    droidCrypto::CommunicationHandler comm(chan, false);
    std::vector<droidCrypto::BitVector> out = circ.evaluate(inVector, DBsize, comm);
    for (int i = 0; i < out.size(); ++i) {
        droidCrypto::Log::v("GC", "Result: %s", out[i].hex().c_str());
    }
    auto totalTimeEnd = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeEnd -totalTimeStart);
    comm.saveBenchmark(comm.TOTAL_TIME, totalTime.count());
    comm.storeLog(inVector.size(), DBsize, 1, k);
}

void Java_com_example_ppil_PPILTask_mainEPPIL(
        JNIEnv *env, jobject /*this*/, jobject channel, jintArray input, jint DBsize, jint rssLength, jint k) {

    auto totalTimeStart = std::chrono::high_resolution_clock::now();
    //Set Parameters
    droidCrypto::Log::v("MA", "Conversion of java values to c++");
    int l = rssLength;
    //Convert java types to c++ types
    jsize N = env->GetArrayLength(input);
    std::vector<int> fingerprint(N);
    env->GetIntArrayRegion(input, jsize{0}, N, &fingerprint[0]);
    int lDash = ceil(log2(pow((pow(2, l) - 1), 2) * N));
    //open Channel
    droidCrypto::Log::v("MA", "Connecting to Server");
    droidCrypto::CSocketChannel chan(ROUTE_IP, ROUTE_PORT, false);
    droidCrypto::Log::v("MA", "Starting offline phase:");
    auto timeOfflineStart = std::chrono::high_resolution_clock::now();
    droidCrypto::Log::v("MA", "Creating Environment");
    droidCrypto::CommunicationHandler comm(chan, false);

    droidCrypto::AEnv aEnv(comm, false, DBsize, fingerprint.size(), lDash);

    comm.prevBytesSent = comm.channel.getBytesSent();
    comm.prevBytesRecv = comm.channel.getBytesRecv();
    //Get database Shares
    droidCrypto::Log::v("MA", "Getting shares from Server");
    auto databaseSharingTimeStart = std::chrono::high_resolution_clock::now();
    aEnv.recvDatabaseShares();

    auto timeOfflineEnd = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> databaseSharingTime = timeOfflineEnd -databaseSharingTimeStart;
    auto databaseSharing = std::chrono::duration_cast<std::chrono::milliseconds>(databaseSharingTime);
    comm.saveBenchmark(comm.AS_SERVER_SHARING_TIME, databaseSharing.count());
    comm.logCommunicationSize(comm.AS_SERVER_SHARING_SENT, comm.AS_SERVER_SHARING_RECV);
    std::chrono::duration<double> offlineTimeAS = timeOfflineEnd -timeOfflineStart;
    droidCrypto::Log::v("MA", "Offline Phase completed in %f seconds.",offlineTimeAS);
    auto offlineAS = std::chrono::duration_cast<std::chrono::milliseconds>(timeOfflineEnd - timeOfflineStart);
    comm.saveBenchmark(comm.OFFLINE_AS_TIME, offlineAS.count());
    comm.logCommunicationSize(comm.OFFLINE_AS_SENT, comm.OFFLINE_AS_RECV);
    droidCrypto::Log::v("MA","Start Online Phase.");
    auto time1 = std::chrono::high_resolution_clock::now();
    //create Fingerprints shares
    droidCrypto::Log::v("MA", "Creating shares");
    aEnv.setFingerprintShares(fingerprint);
    auto time1a = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> fingerprintSharingTime = time1a -time1;
    auto fingerprintSharing = std::chrono::duration_cast<std::chrono::milliseconds>(fingerprintSharingTime);
    comm.saveBenchmark(comm.AS_CLIENT_SHARING_TIME, fingerprintSharing.count());
    comm.logCommunicationSize(comm.AS_CLIENT_SHARING_SENT, comm.AS_CLIENT_SHARING_RECV);
    droidCrypto::Log::v("AS", "Shares sent. Local computations start");
    std::vector<droidCrypto::DeltaShare> distances = computeOpenedEuclidean(aEnv);
    auto time2 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> distanceTime = time2 -time1a;
    auto distanceComp = std::chrono::duration_cast<std::chrono::milliseconds>(distanceTime);
    comm.saveBenchmark(comm.AS_DISTANCE_COMPUTATION_TIME, distanceComp.count());
    comm.logCommunicationSize(comm.AS_DISTANCE_COMPUTATION_SENT, comm.AS_DISTANCE_COMPUTATION_RECV);
    std::chrono::duration<double> timeOnlineAS = time2 -time1;
    auto onlineAS = std::chrono::duration_cast<std::chrono::milliseconds>(timeOnlineAS);
    comm.saveBenchmark(comm.ONLINE_AS_TIME, onlineAS.count());
    droidCrypto::Log::v("MA", "Online time arithmetic sharing: %f seconds", timeOnlineAS);
    comm.logCommunicationSize(comm.ONLINE_AS_SENT, comm.ONLINE_AS_RECV);
    const std::vector<droidCrypto::BitVector> yaoInput = aEnv.toYao(distances);
    chan.clearStats();
    droidCrypto::PPILCircuit circuit(chan, N, distances.size(), k, lDash, lDash);
    std::vector<droidCrypto::BitVector> out = circuit.evaluate(yaoInput, comm);
    for (int i = 0; i < out.size(); ++i) {
        droidCrypto::Log::v("MA", "Index: %s", out[i].hex().c_str());
    }
    auto totalTimeEnd = std::chrono::high_resolution_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeEnd -totalTimeStart);
    comm.saveBenchmark(comm.TOTAL_TIME, totalTime.count());
    comm.storeLog(N,DBsize, l, k);
/*
    for (int i = 0; i < distances.size(); ++i) {
        int resulSshare = comm.recvValue(2);
        int result = distances[i].reconstruction(resulSshare);
        droidCrypto::Log::v("TEST","Share: %s, otherD: %d, Result: %d", distances[i].toString().c_str(), resulSshare, result);

    }

/*
    //Test Logging
    std::vector<std::vector<droidCrypto::DeltaShare>> databaseShares = aEnv.database;
    for (int i = 0; i < DBsize; ++i) {
        std::string logString = "[";
        for (int j = 0; j < aEnv.N; ++j) {
            std::string s = databaseShares[i][j].toString();
            logString.append(s + ", ");
        }
        droidCrypto::Log::v("TEST", "%s]", logString.c_str());
    }
    droidCrypto::Log::v("TEST", "DBSquare Shares");
    vector<vector<droidCrypto::DeltaShare>> dSharesSquare = aEnv.databaseSquared;
    for (int i = 0; i < aEnv.M; ++i) {
        string logString = "[";
        for (int j = 0; j < aEnv.N; ++j) {
            string s = dSharesSquare[i][j].toString();
            logString.append(s + ", ");
        }
        droidCrypto::Log::v("TEST", "%s]", logString.c_str());
    }
    std::vector<std::vector<droidCrypto::DeltaShare>> fingerprintSharesMN = aEnv.fingerprintShares;
    droidCrypto::Log::v("TEST", "fingerprint Shares");
    for (int i = 0; i < DBsize; ++i) {
        std::string logString = "[";
        for (int j = 0; j < aEnv.N; ++j) {
            std::string s = fingerprintSharesMN[i][j].toString();
            logString.append(s + ", ");
        }
        droidCrypto::Log::v("TEST", "%s]", logString.c_str());
    }
    /*std::vector<droidCrypto::DeltaShare> fingerprintShares = aEnv.fingerprint;
    string logstring2 = "[";
    for (int i = 0; i < aEnv.N; ++i) {
        string s = fingerprintShares[i].toString();
        logstring2.append(s + ", ");
    }
    droidCrypto::Log::v("TEST", "%s]", logstring2.c_str());

    droidCrypto::Log::v("TEST", "fingerprintSquare Shares");
    std::vector<droidCrypto::DeltaShare> fingerprintSharesSquare = aEnv.fingerprintSquared;
    string logstring3 = "[";
    for (int i = 0; i < aEnv.N; ++i) {
        string s = fingerprintSharesSquare[i].toString();
        logstring3.append(s + ", ");
    }
    droidCrypto::Log::v("TEST", "%s]", logstring3.c_str());

    vector<vector<int>> mt = aEnv.multiplicationTriplets;
    for (int i = 0; i < aEnv.M; ++i) {
        string logString = "[";
        for (int j = 0; j < aEnv.N; ++j) {
            string s = to_string(mt[i][j]);
            logString.append(s + ", ");
        }
        droidCrypto::Log::v("TEST", "%s]", logString.c_str());
    }
*/
}