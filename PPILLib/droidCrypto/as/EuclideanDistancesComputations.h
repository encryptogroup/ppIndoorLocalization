//
// Created by chris on 30.03.21.
//

#ifndef DROIDCRYPTO_EUCLIDEANDISTANCESCOMPUTATIONS_H
#define DROIDCRYPTO_EUCLIDEANDISTANCESCOMPUTATIONS_H


#include <vector>
#include <droidCrypto/as/DeltaShare.h>
#include "ArithmeticShareEnviroment.h"

using namespace std;

vector<droidCrypto::DeltaShare> computeLocalEuclidean(vector<vector<droidCrypto::DeltaShare>> database, vector<droidCrypto::DeltaShare> fingerprint, droidCrypto::AEnv& aEnv);
vector<droidCrypto::DeltaShare> computeOpenedEuclidean(droidCrypto::AEnv& aEnv);
vector<droidCrypto::DeltaShare> testComputation(droidCrypto::AEnv& aEnv);


#endif //DROIDCRYPTO_EUCLIDEANDISTANCESCOMPUTATIONS_H
