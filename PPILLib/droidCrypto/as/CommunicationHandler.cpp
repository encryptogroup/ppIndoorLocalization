//
// Created by chris on 13.04.21.
//

#include <droidCrypto/utils/Log.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include "CommunicationHandler.h"
#include "ctime"
#include "Config.h"
#include "droidCrypto/utils/Benchmark.h"

void droidCrypto::CommunicationHandler::sendValue(int value, int numberOfBlocks) {
    if(numberOfBlocks > 3){
        uint8_t value1 = value >> 16;
        droidCrypto::block sendingBlock = droidCrypto::toBlock(&value1);
        uint8_t value2 = value >> 8;
        droidCrypto::block sendingBlock1 = droidCrypto::toBlock(&value2);
        uint8_t value3 = value;
        droidCrypto::block sendingBlock2 = droidCrypto::toBlock(&value3);
        std::vector<droidCrypto::block> sendingBlocks{sendingBlock, sendingBlock1, sendingBlock2};
        channel.send(sendingBlocks);
    } else if(numberOfBlocks == 2){
        uint8_t value1 = value >> 8;
        droidCrypto::block paramBlock = droidCrypto::toBlock(&value1);
        uint8_t value2 = value;
        droidCrypto::block paramBlock1 = droidCrypto::toBlock(&value2);
        std::vector<droidCrypto::block> paramBlocks{paramBlock, paramBlock1};
        channel.send(paramBlocks);
    } else{
        uint8_t value1 = value;
        droidCrypto::block sendingBlock = droidCrypto::toBlock(&value1);
        channel.send(sendingBlock);
    }
}

int droidCrypto::CommunicationHandler::recvValue(int numberOfBlocks) {
    if(numberOfBlocks == 1){
        droidCrypto::block receivingBlock;
        channel.recv(receivingBlock);
        uint8_t *value = (uint8_t *) &receivingBlock;
        return *value;
    }
    std::vector<droidCrypto::block> recvBlocks(numberOfBlocks);
    channel.recv(recvBlocks);
    int value = 0;
    std::vector<uint8_t> byteBlocks;
    for (int i = 0; i < numberOfBlocks; ++i) {
        uint8_t *x = (uint8_t *) &recvBlocks[i];
        byteBlocks.push_back(*x);
    }
    switch (numberOfBlocks) {
        case 3:
            value = (byteBlocks[0] << 16) | byteBlocks[1] << 8 | byteBlocks[2];
            break;
        case 2:
            value = (byteBlocks[0] << 8) | byteBlocks[1];
            break;
        default:
            break;
    }
    return value;
}

void droidCrypto::CommunicationHandler::logCommunicationSize(const Benchmark sending, const Benchmark receiving){
    int recvBytes = channel.getBytesRecv() - prevBytesRecv;
    int sendBytes = channel.getBytesSent() - prevBytesSent;
    droidCrypto::Log::v("CH","Bytes sent: %d", sendBytes);
    droidCrypto::Log::v("CH", "Bytes received: %d", recvBytes);
    saveBenchmark(sending , sendBytes);
    saveBenchmark(receiving, recvBytes);
    prevBytesRecv = channel.getBytesRecv();
    prevBytesSent = channel.getBytesSent();
}

void droidCrypto::CommunicationHandler::sendValues(std::vector<int> values, int numberOfBlocks) {
    std::vector<droidCrypto::block> sendingBlocks;
    std::vector<uint8_t> valueBlocks;
    for (int i = 0; i < values.size(); ++i) {
        std::vector<uint8_t> tmp = packBlocks(values[i], numberOfBlocks);
        if (valueBlocks.size() > 16 - numberOfBlocks){
            droidCrypto::block b = droidCrypto::toBlock(&valueBlocks[0]);
            //droidCrypto::Log::v("Testi",b);
            sendingBlocks.push_back(b);
            valueBlocks.clear();
        }
        for (int j = 0; j < tmp.size(); ++j) {
            valueBlocks.push_back(tmp[j]);
        }
    }
    droidCrypto::block b = droidCrypto::toBlock(&valueBlocks[0]);
    //droidCrypto::Log::v("Testi",b);
    sendingBlocks.push_back(b);
    valueBlocks.clear();
    channel.send(sendingBlocks);
}

std::vector<int> droidCrypto::CommunicationHandler::recvValues(int numberOfValues, int numberOfBlocks) {
    int vectorsize = std::ceil(((float)numberOfBlocks * (float)numberOfValues)/16);
    std::vector<droidCrypto::block> recvBlocks(vectorsize);
    std::vector<int> values;
    channel.recv(recvBlocks);
    //droidCrypto::Log::v("TestO", recvBlocks[0]);
    for (int i = 0; i < recvBlocks.size(); ++i) {
        uint8_t *blockbegin = ByteArray(recvBlocks[i]);
        for (int j = 0; j < 17 - numberOfBlocks; ++j) {
            int value = unpackBlocks(blockbegin, numberOfBlocks, j);
            values.push_back(value);
            j = j + numberOfBlocks - 1;
        }
    }
    return values;
}

std::vector<uint8_t> droidCrypto::CommunicationHandler::packBlocks(int value, int numberOfBlocks) {
    if(numberOfBlocks == 3){
        uint8_t value1 = value >> 16;
        uint8_t value2 = value >> 8;
        uint8_t value3 = value;
        std::vector<uint8_t> sendingBlocks{value1, value2, value3};
        return sendingBlocks;
    } else if(numberOfBlocks == 2){
        uint8_t value1 = value >> 8;
        uint8_t value2 = value;
        std::vector<uint8_t> paramBlocks{value1, value2};
        return paramBlocks;
    } else{
        uint8_t value1 = value;
        return {value1};
    }
}

int droidCrypto::CommunicationHandler::unpackBlocks(uint8_t *blockPointer, int numberOfBlocks, int blockCnt) {
    int value = 0;
    for (int i = 0; i < numberOfBlocks; ++i) {

        value = value | (blockPointer[blockCnt + i] << (numberOfBlocks - 1 - i)*8);
    }
    return value;
}

int droidCrypto::CommunicationHandler::unpackBlocks(std::vector<droidCrypto::block> blocks, int numberOfBlocks, int numberOfValues) {
    int value = 0;
    int x = 16;
    int valueCnt = 0;

    for (int i = 0; i < blocks.size(); ++i) {
        uint8_t *startBytesBlock = droidCrypto::ByteArray(blocks[i]);
        for (int j = 0; j < x - (numberOfBlocks - 1); ++j) {
            switch (numberOfBlocks) {
                case 3:
                    value = (startBytesBlock[j] << 16) | startBytesBlock[j+1] << 8 | startBytesBlock[j+2];
                    j = j + 2;
                    break;
                case 2:
                    value = (startBytesBlock[j] << 8) | startBytesBlock[j+1];
                    j = j + 1;
                    break;
                case 1:
                    value = startBytesBlock[j];
                default:
                    break;
            }
            valueCnt++;
        }
        if(valueCnt + (16/numberOfBlocks) >= numberOfValues){
            x = numberOfValues - (valueCnt + (16/numberOfBlocks));
        }
    }
    return value;
}

int droidCrypto::CommunicationHandler::unpackBlocks(std::vector<droidCrypto::block> blocks, int numberOfBlocks) {
    int value = 0;
    std::vector<uint8_t> byteBlocks;
    for (int i = 0; i < numberOfBlocks; ++i) {
        uint8_t *x = (uint8_t *) &blocks[i];
        byteBlocks.push_back(*x);
    }
    switch (numberOfBlocks) {
        case 3:
            value = (byteBlocks[0] << 16) | byteBlocks[1] << 8 | byteBlocks[2];
            break;
        case 2:
            value = (byteBlocks[0] << 8) | byteBlocks[1];
            break;
        case 1:
            value = byteBlocks[0];
        default:
            break;
    }
    return value;
}

droidCrypto::block droidCrypto::CommunicationHandler::packBlock(int value){
    std::array<uint8_t, 2> s1ByteArray;
    s1ByteArray[0] = value >> 8;
    s1ByteArray[1] = value;
    block s1Block = toBlock(&s1ByteArray[0]);
}

int droidCrypto::CommunicationHandler::unpackBlock(droidCrypto::block value, int relevantBytes) {
    uint8_t *blockPointer = ByteArray(value);
    int res = 0;
    for (int i = 0; i < relevantBytes; ++i) {
        res = res | blockPointer[i] << ((relevantBytes-1-i)*8);
    }
    return res;
}

void droidCrypto::CommunicationHandler::storeLog(int N, int M, int L, int k) {
    if (!isServer) {
        std::vector<int> sendingVec(std::begin(logInformation), std::end(logInformation));
        sendValues(sendingVec, 3);
    } else {
        std::vector<int> clientLogs = recvValues(logInformation.size(), 3);
        std::string filename = DATBASE_PATH;
        filename.append("Logs/");
        if (L == 1) {
            filename.append("RSS1/");
        } else {
            filename.append("RSS4/");
        }

        filename.append("N");
        filename.append(std::to_string(N));
        filename.append("M");
        filename.append(std::to_string(M));
        filename.append("L");
        filename.append(std::to_string(L));
        filename.append("K");
        filename.append(std::to_string(k));
        filename.append(".csv");

        //std::string line;
        std::ifstream fileCheck(filename);
        std::ofstream file;
        if (!fileCheck.is_open()) {
            file.open(filename);
            if (file.is_open()) {
                file << "Party" ;
                for (int i = 1; i < logTags.size(); ++i) {
                    file << "," << logTags[i];
                }
                file << "\n";
                file.close();
            } else {
                throw std::logic_error("File could not be opened1");
            }
        }
        file.open(filename, std::ios::app);
        if (file.is_open()) {
            //file << "Server";
            //for (int i = 1; i < logInformation.size(); ++i) {
            //    file << "," << logInformation[i];
            //}
            //file << "\n";
            file << "Client";
            for (int i = 1; i < logInformation.size(); ++i) {
                if (i == GC_GARBLING_TIME.value){
                    file << "," << logInformation[i];
                } else {
                    file << "," << clientLogs[i];
                }
            }
            file << "\n";
        } else {
            throw std::logic_error("File could not be opened");
        }
    }
}

void droidCrypto::CommunicationHandler::saveBenchmark(Benchmark benchmark, int benchmarkValue) {
    if (!(benchmark.value == NOT_IN_CSV)){
        logInformation[benchmark.value] = benchmarkValue;
        logTags[benchmark.value] = benchmark.name;
    }
}
