#pragma once
// This file and the associated implementation has been placed in the public
// domain, waiving all copyright. No restrictions are placed on its use.
#include <droidCrypto/Defines.h>
#include <array>
#include <memory>
#include <droidCrypto/utils/Log.h>
#include <droidCrypto/as/CommunicationHandler.h>
#include "droidCrypto/ChannelWrapper.h"

namespace droidCrypto {
static const uint64_t commStepSize(512);
static const uint64_t superBlkSize(8);

class PRNG;
class BitVector;
class ChannelWrapper;

// The hard coded number of base OT that is expected by the OT Extension
// implementations. This can be changed if the code is adequately adapted.
const uint64_t gOtExtBaseOtCount(128);

class OtReceiver {
 public:
  OtReceiver() = default;
  virtual ~OtReceiver() = default;

  virtual void receive(const BitVector &choices, span<block> messages,
                       PRNG &prng, ChannelWrapper &chan) = 0;

    void receiveChosen(const BitVector &choices, span <block> recvMessages, PRNG &prng, ChannelWrapper &chl);

    std::vector<int> recvValues(ChannelWrapper &chl, int numberOfValues, int numberOfBlocks);

    int unpackBlocks(uint8_t *blockPointer, int numberOfBlocks, int blockCnt);

    block packBlock(int value);

    void receiveCorrelated(int M, int N, const std::string& choiceString, span <block> recvMessages, PRNG &prng,
                           ChannelWrapper &chl);
};

class OtSender {
 public:
  OtSender() = default;
  virtual ~OtSender() = default;

  virtual void send(span<std::array<block, 2>> messages, PRNG &prng,
                    ChannelWrapper &chan) = 0;
    void sendChosen(span<std::array<block, 2>> recvMessages, PRNG &prng, ChannelWrapper &chl);


    void sendCorrelated(span<block> messages, std::vector<int> shares, int M, int N, int l, int ringSize, PRNG& prng, ChannelWrapper &chl)
    {
        std::vector<block> randomOTs;
        randomOTs.resize(N*l);
        std::vector<std::array<block, 2>> temp(randomOTs.size());
        std::vector<block> temp2(messages.size());
        droidCrypto::Log::v("", "tmp size: %d", temp.size());
        send(temp, prng, chl);
        std::vector<int> sendingValues;
        std::vector<block> sendingBlocks;
        int bitIterator = 0;
        int shareIterator = 0;
        int j = 0;
        for (uint64_t i = 0; i < static_cast<uint64_t>(messages.size()); ++i)
        {
            if(bitIterator > l-1){
                bitIterator = 0;
                shareIterator++;
            }
            if (j >= randomOTs.size()){
                j = 0;
            }
            messages[i] = temp[j][0];
            temp2[i] = temp[j][1] ^ correlatedFunction(shares[shareIterator], temp[j][0], bitIterator, ringSize);
            //droidCrypto::Log::v("Test", temp2[i]);
             int tmpInt = unpackBlock(temp2[i],2);
             sendingValues.push_back(tmpInt);
             sendingBlocks.push_back(temp2[i]);
             bitIterator++;
             j++;
        }
        sendValues(chl, sendingValues, 2);
        //com.channel.send(sendingBlocks);
    }

    block correlatedFunction(long share, droidCrypto::block &input, int bitIteration, int ringSize) const;
    //int correlatedFunction(long share, droidCrypto::block &input, int bitIteration, int ringSize) const;



    int unpackBlock(block value, int relevantBytes);

    std::vector<uint8_t> packBlocks(int value, int numberOfBlocks);

    void sendValues(ChannelWrapper &chl, std::vector<int> values, int numberOfBlocks);
};


class OtExtReceiver : public OtReceiver {
 public:
  OtExtReceiver() = default;
  virtual ~OtExtReceiver() = default;

  virtual void setBaseOts(span<std::array<block, 2>> baseSendOts) = 0;

  virtual bool hasBaseOts() const = 0;
  virtual std::unique_ptr<OtExtReceiver> split() = 0;

};

class OtExtSender : public OtSender {
 public:
  OtExtSender() = default;
  virtual ~OtExtSender() = default;

  virtual bool hasBaseOts() const = 0;

  virtual void setBaseOts(span<block> baseRecvOts,
                          const BitVector &choices) = 0;

  virtual std::unique_ptr<OtExtSender> split() = 0;
};

}  // namespace droidCrypto
