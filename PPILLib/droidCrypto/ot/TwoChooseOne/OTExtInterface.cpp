//
// Created by chris on 11.05.21.
//

#include "OTExtInterface.h"
#include <droidCrypto/BitVector.h>
#include <vector>
#include <droidCrypto/ChannelWrapper.h>
#include <cassert>

void droidCrypto::OtSender::sendChosen(
        span<std::array<block, 2>> messages,
        PRNG & prng,
        ChannelWrapper &chl)
{
    std::vector<std::array<block, 2>> temp(messages.size());
    send(temp, prng, chl);
    std::vector<block> sendingBlocks;

    for (uint64_t i = 0; i < static_cast<uint64_t>(messages.size()); ++i)
    {
        temp[i][0] = temp[i][0] ^ messages[i][0];
        temp[i][1] = temp[i][1] ^ messages[i][1];
        sendingBlocks.push_back(temp[i][0]);
        sendingBlocks.push_back(temp[i][1]);
    }

    chl.send(sendingBlocks);
}

//int
droidCrypto::block
droidCrypto::OtSender::correlatedFunction(long share, droidCrypto::block &input, int bitIteration, int ringSize) const {
    uint8_t *s0Pointer = ByteArray(input);
    int s0 = s0Pointer[0] << 8 | s0Pointer[1];
    //droidCrypto::Log::v("Test", input);
    //droidCrypto::Log::v("Test","blockvalue %d",s0);
    //int s0 = s0Pointer[0];
    s0 = s0 % ringSize;
    int s1 = positive_modulo(ringSize, ((share * pow(2, bitIteration)) + s0));
    //return s1;
    std::array<uint8_t, 16> s1ByteArray;
    s1ByteArray[0] = s1 >> 8;
    s1ByteArray[1] = s1;
    for (int i = 2; i < 16; ++i) {
        s1ByteArray[i] = 0;
    }
    block s1Block = toBlock(&s1ByteArray[0]);
    return s1Block;
}

void droidCrypto::OtReceiver::receiveChosen(
        const BitVector & choices,
        span<block> recvMessages,
        PRNG & prng,
        ChannelWrapper &chl)
{
    receive(choices, recvMessages, prng, chl);
    std::vector<std::array<block,2>> temp(recvMessages.size());
    std::vector<block> recvBlocks(temp.size()*2);
    chl.recv(recvBlocks);
    auto iter = choices.begin();
    int j = 0;
    for (uint64_t i = 0; i < temp.size(); ++i)
    {
        temp[i][0] = recvBlocks[j];
        j++;
        temp[i][1] = recvBlocks[j];
        j++;
        recvMessages[i] = recvMessages[i] ^ temp[i][*iter];
        ++iter;
    }
}

void droidCrypto::OtReceiver::receiveCorrelated(int M, int N, const std::string& choiceString, span<block> recvMessages, PRNG& prng, ChannelWrapper &chl)
{
    //assert(choices.size() == recvMessages.size());
    BitVector choiceBits = BitVector(choiceString);
    std::vector<block> randomOTs;
    randomOTs.resize(choiceBits.size());
    std::string fullChoiceString = choiceString;
    for (int i = 1; i < M; ++i) {
        fullChoiceString.append(choiceString);
    }
    BitVector choices = BitVector(fullChoiceString);

    receive(choiceBits, span<block>(randomOTs.data(), randomOTs.size()), prng, chl);
    //droidCrypto::Log::v("Test", "choices: %s", choices.hex().c_str());
    std::vector<block> temp(recvMessages.size());
    std::vector<block> recvBlocks(temp.size());
    std::vector<int> corValues = recvValues(chl, recvBlocks.size(), 2);
    auto iter = choices.begin();
    int j = 0;
    for (uint64_t i = 0; i < temp.size(); ++i)
    {
        if (j >= choiceString.size()){
            j = 0;
        }
        block recvBlock = packBlock(corValues[i]);
        temp[i] = recvBlock;
        recvMessages[i] = randomOTs[j] ^ (ZeroAndAllOneBlock[*iter] & temp[i]);
        //droidCrypto::Log::v("Test", ZeroAndAllOneBlock[*iter]);
        //droidCrypto::Log::v("Test", recvMessages[i]);
        ++iter;
        j++;
    }

}
droidCrypto::block droidCrypto::OtReceiver::packBlock(int value){
    std::array<uint8_t, 16> s1ByteArray{};
    s1ByteArray[0] = value >> 8;
    s1ByteArray[1] = value;
    for (int i = 2; i < 16; ++i) {
        s1ByteArray[i] = 0;
    }
    block s1Block = toBlock(&s1ByteArray[0]);
    return s1Block;
}

std::vector<int> droidCrypto::OtReceiver::recvValues(ChannelWrapper &chl, int numberOfValues, int numberOfBlocks) {
    int vectorsize = std::ceil(((float)numberOfBlocks * (float)numberOfValues)/16);
    std::vector<droidCrypto::block> recvBlocks(vectorsize);
    std::vector<int> values;
    chl.recv(recvBlocks);
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

int droidCrypto::OtReceiver::unpackBlocks(uint8_t *blockPointer, int numberOfBlocks, int blockCnt) {
    int value = 0;
    for (int i = 0; i < numberOfBlocks; ++i) {

        value = value | (blockPointer[blockCnt + i] << (numberOfBlocks - 1 - i)*8);
    }
    return value;
}

int droidCrypto::OtSender::unpackBlock(droidCrypto::block value, int relevantBytes) {
    uint8_t *blockPointer = ByteArray(value);
    int res = 0;
    for (int i = 0; i < relevantBytes; ++i) {
        res = res | blockPointer[i] << ((relevantBytes-1-i)*8);
    }
    return res;
}

void droidCrypto::OtSender::sendValues(ChannelWrapper &chl, std::vector<int> values, int numberOfBlocks) {
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
    chl.send(sendingBlocks);
}

std::vector<uint8_t> droidCrypto::OtSender::packBlocks(int value, int numberOfBlocks) {
    if(numberOfBlocks > 3){
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
