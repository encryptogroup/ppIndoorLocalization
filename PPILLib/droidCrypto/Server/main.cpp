#include <iostream>
#include <atomic>
#include <vector>
#include <string>
#include <droidCrypto/BitVector.h>
#include <droidCrypto/gc/circuits/PPILCircuit.h>
#include "Config.h"
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <droidCrypto/as/DeltaShare.h>
#include <droidCrypto/utils/Log.h>
#include <droidCrypto/loguru.cpp>
#include <droidCrypto/as/CommunicationHandler.h>
#include <droidCrypto//as/EuclideanDistancesComputations.h>
#include "../Defines.h"

using namespace std;

atomic_flag ready;

vector<droidCrypto::block> preparePramSending(int param){

        uint8_t value = param >> 8;
        droidCrypto::block paramBlock = droidCrypto::toBlock(&value);
        uint8_t value1 = param;
        droidCrypto::block paramBlock1 = droidCrypto::toBlock(&value1);
        vector<droidCrypto::block> paramBlocks{paramBlock, paramBlock1};
        return paramBlocks;
}

int main(int argc, char **argv) {
    loguru::init(argc, argv);
    loguru::add_file("PPILServer.saveLog", loguru::Append, loguru::Verbosity_MAX);

    if(argc < 4){
        LOG_F(ERROR, "Please provide database path, number of the protocol and k");
        return 141;
    }

    uint16_t N;
    uint16_t M;
    uint8_t l;
    uint8_t protocolNum = atoi(argv[2]);
    if(protocolNum > 1 || protocolNum < 0){
        LOG_F(ERROR, "The protocol number must be 0 or 1");
    }
    const uint8_t k = atoi(argv[3]);
    string filename = DATBASE_PATH;
    filename.append(argv[1]);
    string line;
    ifstream file (filename);
    vector<vector<int>> database;
    vector<droidCrypto::BitVector> RSSValues;
    LOG_F(INFO,"Load Database");
    if (file.is_open())
    {
        while ( getline (file, line) )
        {
            std::vector<string> vec;
            boost::algorithm::split(vec, line, boost::is_any_of(","));
            N = vec.size();
            l = vec[0].size();
            if(vec[0].size() == 1 && protocolNum == 0) {
                string vi;
                for(int i = 0; i < N; i++){
                       vi.append(vec[i]);
                }
                RSSValues.push_back(droidCrypto::BitVector(vi));
            } else {
                vector<int> fingerprint;
                string logString = "[";
                for (int i = 0; i < N; i++) {
                    char *end;
                    uint8_t vi = strtoul(vec[i].c_str(),&end, 2);
                    fingerprint.push_back(vi);
                    logString.append(std::to_string(vi) + ", ");
                }
                LOG_F(INFO, "Database value of %s: %s", line.c_str(), (logString.append("]")).c_str());
                database.push_back(fingerprint);
            }
            if(protocolNum == 0){
                M = RSSValues.size();
            } else{
                M = database.size();
            }
        }
        file.close();
    } else{
        cout << "No Database-File found at " << filename;
        return -1;
    }
    LOG_F(INFO, "Database loaded with M = %d, N = %d, l = %d, k = %d, p = %d", M, N, l, k, protocolNum);

    //sending params to client
    vector<droidCrypto::block> parameterM = preparePramSending(M);
    vector<droidCrypto::block> parameterN = preparePramSending(N);
    droidCrypto::block parameterL = droidCrypto::toBlock(&l);
    droidCrypto::block parameterK = droidCrypto::toBlock(&k);
    droidCrypto::block parameterP = droidCrypto::toBlock(&protocolNum);

    droidCrypto::CSocketChannel chan(ROUTE_IP, ROUTE_PORT, true);

    chan.send(parameterM);
    chan.send(parameterN);
    chan.send(parameterL);
    chan.send(parameterK);
    chan.send(parameterP);
    LOG_F(INFO, "protocol parameters sent to Client");

    chan.closeChannel();
    if(protocolNum == 0){
        //Distance computation with Manhattan
        for(int i = 1; i > 0; i++){
            LOG_F(INFO, "Connecting to Device");
            droidCrypto::CSocketChannel chan(ROUTE_IP, ROUTE_PORT, true);
            auto totalTimeStart = std::chrono::high_resolution_clock::now();
            droidCrypto::CommunicationHandler communicationHandler(chan, true);
            LOG_F(INFO, "garble circuit");
            droidCrypto::PPILCircuit circ(chan, N, M, k, l, N);
            circ.garble(RSSValues, 0, communicationHandler);
            LOG_F(INFO, "%d. Localisation done", i);
            auto totalTimeEnd = std::chrono::high_resolution_clock::now();
            auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeEnd - totalTimeStart);
            communicationHandler.saveBenchmark(communicationHandler.TOTAL_TIME, totalTime.count());
            LOG_F(INFO,"Total time in %d milliseconds. Garbling circuit", totalTime.count());
            communicationHandler.storeLog(N,M, l,k);
        }
    } else{
        //Distance computation with Euclidean
        int lDash = ceil(log2(pow((pow(2, l) - 1), 2) * N));
        LOG_F(INFO, "%d", lDash);
        for (int ui = 1; ui > 0; ++ui) {
            LOG_F(INFO, "Connecting to Device");
            droidCrypto::CSocketChannel chan(ROUTE_IP, ROUTE_PORT, true);
            auto totalTimeStart = std::chrono::high_resolution_clock::now();
            LOG_F(INFO,"Start Offline time");
            auto timeOfflineStart = std::chrono::high_resolution_clock::now();
            LOG_F(INFO, "Creating Environment");
            droidCrypto::CommunicationHandler comm(chan, true);

            droidCrypto::AEnv aEnv(comm, true, M, N, lDash);
            comm.prevBytesRecv = comm.channel.getBytesRecv();
            comm.prevBytesSent = comm.channel.getBytesSent();
            //create shares for database values
            LOG_F(INFO, "Creating Shares for database values");
            auto databaseSharingTimeStart = std::chrono::high_resolution_clock::now();

            aEnv.setDatabaseShares(database);

            auto timeOfflineEnd = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> databaseSharingTime = timeOfflineEnd -databaseSharingTimeStart;
            auto databaseSharing = std::chrono::duration_cast<std::chrono::milliseconds>(databaseSharingTime);
            comm.saveBenchmark(comm.AS_SERVER_SHARING_TIME, databaseSharing.count());
            comm.logCommunicationSize(comm.AS_SERVER_SHARING_SENT, comm.AS_SERVER_SHARING_RECV);

            std::chrono::duration<double, std::milli> offlineTimeAS = timeOfflineEnd -timeOfflineStart;
            auto offlineAS = std::chrono::duration_cast<std::chrono::milliseconds>(timeOfflineEnd - timeOfflineStart);
            LOG_F(INFO,"Offline phase completed in %f milliseconds.",offlineTimeAS);
            comm.saveBenchmark(comm.OFFLINE_AS_TIME, offlineAS.count());
            comm.logCommunicationSize(comm.OFFLINE_AS_SENT, comm.OFFLINE_AS_RECV);
            LOG_F(INFO,"Waiting for Client input");
            auto timeOnlineStart = std::chrono::high_resolution_clock::now();
            //wait for fingerprint shares
            aEnv.recvFingerprintShares();

            auto time2 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> fingerprintSharingTime = time2 - timeOnlineStart;
            auto fingerprintSharing = std::chrono::duration_cast<std::chrono::milliseconds>(fingerprintSharingTime);
            comm.saveBenchmark(comm.AS_CLIENT_SHARING_TIME, fingerprintSharing.count());
            comm.logCommunicationSize(comm.AS_CLIENT_SHARING_SENT, comm.AS_CLIENT_SHARING_RECV);

            LOG_F(INFO, "Received fingerprint deltas and created shares. Starting with computing of distances");
            vector<droidCrypto::DeltaShare> distances;
            distances = computeOpenedEuclidean(aEnv);
            auto time3 = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> computeDistanceTime = time3 - time2;
            auto computeDistance = std::chrono::duration_cast<std::chrono::milliseconds>(computeDistanceTime);
            comm.saveBenchmark(comm.AS_DISTANCE_COMPUTATION_TIME, computeDistance.count());
            comm.logCommunicationSize(comm.AS_DISTANCE_COMPUTATION_SENT, comm.AS_DISTANCE_COMPUTATION_RECV);

            std::vector<droidCrypto::BitVector> yaoInput = aEnv.toYao(distances);
            auto timeOnlineEnd = std::chrono::high_resolution_clock::now();

            std::chrono::duration<double> yaoInputTime = timeOnlineEnd - time3;
            auto yaoInputTimeAS = std::chrono::duration_cast<std::chrono::milliseconds>(yaoInputTime);
            comm.saveBenchmark(comm.AS_SERVER_INPUT_PREP_TIME, yaoInputTimeAS.count());
            comm.logCommunicationSize(comm.AS_SERVER_INPUT_PREP_SENT, comm.AS_SERVER_INPUT_PREP_RECV);


            std::chrono::duration<double> onlineTimeAS = timeOnlineEnd -timeOnlineStart;
            auto onlineAS = std::chrono::duration_cast<std::chrono::milliseconds>(timeOnlineEnd -timeOnlineStart);
            comm.saveBenchmark(comm.ONLINE_AS_TIME, onlineAS.count());
            LOG_F(INFO,"AS Online phase completed in %f seconds. Garbling circuit", onlineTimeAS);
            comm.logCommunicationSize(comm.ONLINE_AS_SENT, comm.ONLINE_AS_RECV);
            chan.clearStats();
            droidCrypto::PPILCircuit circuit(chan, N, M, k, lDash, lDash);
            circuit.garble(yaoInput, 1, comm);
            auto totalTimeEnd = std::chrono::high_resolution_clock::now();
            auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(totalTimeEnd -totalTimeStart);
            comm.saveBenchmark(comm.TOTAL_TIME, totalTime.count());
            LOG_F(INFO,"Total time in %d milliseconds. Garbling circuit", totalTime.count());
            comm.storeLog(N, M, l, k);
/*
            for (int i = 0; i < distances.size(); ++i) {
                //LOG_F(INFO, "%s", distances[i].toString().c_str());
                int share = distances[i].getSmallDeltaShare();
                comm.sendValue(share, 2);
            }


          /*  vector<vector<droidCrypto::DeltaShare>> dShares = aEnv.database;
            for (int i = 0; i < M; ++i) {
                string logString = "[";
                for (int j = 0; j < N; ++j) {
                    string s = dShares[i][j].toString();
                    logString.append(s + ", ");
                }
                LOG_F(INFO, "%s]", logString.c_str());
            }
            LOG_F(INFO, "DBSquare Shares");
            vector<vector<droidCrypto::DeltaShare>> dSharesSquare = aEnv.databaseSquared;
            for (int i = 0; i < M; ++i) {
                string logString = "[";
                for (int j = 0; j < N; ++j) {
                    string s = dSharesSquare[i][j].toString();
                    logString.append(s + ", ");
                }
                LOG_F(INFO, "%s]", logString.c_str());
            }
            LOG_F(INFO, "fingerprint Shares");
            vector<vector<droidCrypto::DeltaShare>> fingerprintSharesMN = aEnv.fingerprintShares;
            for (int i = 0; i < M; ++i) {
                string logString = "[";
                for (int j = 0; j < N; ++j) {
                    string s = fingerprintSharesMN[i][j].toString();
                    logString.append(s + ", ");
                }
                LOG_F(INFO, "%s]", logString.c_str());
            }
            /*std::vector<droidCrypto::DeltaShare> fingerprintShares = aEnv.fingerprint;
            string logstring3 = "[";
            for (int i = 0; i < N; ++i) {
                string s = fingerprintShares[i].toString();
                logstring3.append(s + ", ");
            }
            LOG_F(INFO, "%s]", logstring3.c_str());

            LOG_F(INFO, "fingerprintSquare Shares");
            std::vector<droidCrypto::DeltaShare> fingerprintSharesSquare = aEnv.fingerprintSquared;
            string logstring2 = "[";
            for (int i = 0; i < N; ++i) {
                string s = fingerprintSharesSquare[i].toString();
                logstring2.append(s + ", ");
            }
            LOG_F(INFO, "%s]", logstring2.c_str());

            vector<vector<int>> mt = aEnv.multiplicationTriplets;
            for (int i = 0; i < M; ++i) {
                string logString = "[";
                for (int j = 0; j < N; ++j) {
                    string s = to_string(mt[i][j]);
                    logString.append(s + ", ");
                }
                LOG_F(INFO, "%s]", logString.c_str());
            }
*/
        }
    }

    return 0;
}

