//
// Created by chris on 07.07.21.
//
#ifndef DROIDCRYPTO_BENCHMARK_H
#define DROIDCRYPTO_BENCHMARK_H

namespace droidCrypto {

    class Benchmark {
    public:
        Benchmark(std::string tag, int number){
            name = tag;
            value = number;
        }
        std::string name;
        int value;
    };
}

#endif