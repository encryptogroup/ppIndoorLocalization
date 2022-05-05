//
// Created by chris on 26.03.21.
//

#ifndef DROIDCRYPTO_DELTASHARE_H
#define DROIDCRYPTO_DELTASHARE_H


#include <cstdint>
#include <cstdio>
#include <cmath>
#include <string>

namespace droidCrypto{
    class DeltaShare{
    public:
        DeltaShare(int seed, size_t length);
        DeltaShare(int bigDelta, int smallDeltaS, size_t ringsize);

        int reconstruction(int otherSDelta);
        std::string toString() const;
        int getCapitalDelta() const{return capitalDelta;};
        int getSmallDeltaShare() const{return smallDeltaShare;};
        size_t getRingSize() const{return ringSize;};
        void setCapitalDelta(int bigDelta);

        DeltaShare operator+(const DeltaShare &other) const;
        DeltaShare operator+(const int &other) const;
        DeltaShare operator-(const DeltaShare &other) const;
        DeltaShare operator-(const int &other) const;
        DeltaShare operator*(const int &other) const;

        void setValue(int value, int seed);

    private:
        int capitalDelta;
        int smallDeltaShare;
        size_t ringSize;
        bool isSet;

    };
}


#endif //DROIDCRYPTO_DELTASHARE_H
