//
// Created by chris on 26.03.21.
//

#include "DeltaShare.h"
#include "ArithmeticShareEnviroment.h"
#include <cstdlib>
#include <ctime>
#include <droidCrypto/utils/Log.h>

namespace droidCrypto{

    #define ERROR_MSG_NOT_INITIALIZED "Share has not been fully initialized"
    #define ERROR_MSG_ALREADY_INITIALIZED "share already is initialized with a value"

    DeltaShare::DeltaShare(int seed, size_t ring){
        ringSize = ring;
        srand(seed);
        smallDeltaShare = rand() %  ringSize;
        isSet = false;
    }

    DeltaShare::DeltaShare(int bigDelta, int smallDeltaS, size_t ringsize){
        ringSize = ringsize;
        capitalDelta = bigDelta;
        smallDeltaShare = smallDeltaS;
        isSet = true;
    }

    void DeltaShare::setValue(int value, int seed){
        /*if(isSet){
            Log::e("AS", ERROR_MSG_ALREADY_INITIALIZED);
            throw std::runtime_error(ERROR_MSG_ALREADY_INITIALIZED);
        }*/
        srand(seed);
        int otherSDelta =  rand() % ringSize;
        capitalDelta = positive_modulo(ringSize,value + smallDeltaShare + otherSDelta);
        isSet = true;
    }

    int DeltaShare::reconstruction(int otherSDelta) {
        if(!isSet){
            Log::e("AS", ERROR_MSG_NOT_INITIALIZED);
            throw std::runtime_error(ERROR_MSG_NOT_INITIALIZED);
        }
        return droidCrypto::positive_modulo(ringSize, (capitalDelta - smallDeltaShare - otherSDelta));
    }

    DeltaShare DeltaShare::operator+(const DeltaShare &other) const {
        if(!(isSet || other.isSet)){
            Log::e("AS", ERROR_MSG_NOT_INITIALIZED);
            throw std::runtime_error(ERROR_MSG_NOT_INITIALIZED);
        }
        int bigDelta = positive_modulo(ringSize, capitalDelta + other.capitalDelta);
        int smallDeltaS = positive_modulo(ringSize, smallDeltaShare + other.smallDeltaShare);
        DeltaShare result = DeltaShare(bigDelta, smallDeltaS, ringSize);
        return result;
    }

    DeltaShare DeltaShare::operator-(const DeltaShare &other) const {
        if(!(isSet || other.isSet)){
            Log::e("AS", ERROR_MSG_NOT_INITIALIZED);
            throw std::runtime_error(ERROR_MSG_NOT_INITIALIZED);
        }
        int bigDelta = positive_modulo(ringSize,capitalDelta - other.capitalDelta);
        int smallDeltaS = positive_modulo(ringSize, smallDeltaShare - other.smallDeltaShare);
        DeltaShare result = DeltaShare(bigDelta, smallDeltaS, ringSize);
        return result;
    }

    DeltaShare DeltaShare::operator*(const int &other) const {
        if(!isSet){
            Log::e("AS", ERROR_MSG_NOT_INITIALIZED);
            throw std::runtime_error(ERROR_MSG_NOT_INITIALIZED);
        }
        int bigDelta = (capitalDelta * other) % ringSize;
        int smallDeltaS = (smallDeltaShare * other) % ringSize;
        DeltaShare result = DeltaShare(bigDelta, smallDeltaS, ringSize);
        return result;
    }

    std::string DeltaShare::toString() const {
        if(!isSet){
            Log::e("AS", ERROR_MSG_NOT_INITIALIZED);
            throw std::runtime_error(ERROR_MSG_NOT_INITIALIZED);
        }
        std::string s;
        s = "(D: " + std::to_string(capitalDelta) + ", d[i]: " + std::to_string(smallDeltaShare) + ")" ;
        return s;
    }

    void DeltaShare::setCapitalDelta(int bigDelta) {
        //if (isSet){
        //    Log::e("AS", ERROR_MSG_ALREADY_INITIALIZED);
        //    throw std::runtime_error(ERROR_MSG_ALREADY_INITIALIZED);
       // }
        capitalDelta = bigDelta;
        isSet = true;
    }
}