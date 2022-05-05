#include <droidCrypto/ChannelWrapper.h>
#include <droidCrypto/BitVector.h>
#include <droidCrypto/gc/circuits/PPILCircuit.h>

//
// Created by chris on 09.12.20.
//

std::atomic_flag ready;

int main(){

    droidCrypto::CSocketChannel chan("127.0.0.1", 8000, true);
    droidCrypto::BitVector inVector("11111111");
    droidCrypto::PPILCircuit circ(chan, inVector.size());
    circ.garble(inVector);
    return 0;
}
