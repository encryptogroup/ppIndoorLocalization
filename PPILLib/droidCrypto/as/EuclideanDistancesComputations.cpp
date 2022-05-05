//
// Created by chris on 30.03.21.
//

#include <droidCrypto/utils/Log.h>
#include "EuclideanDistancesComputations.h"



vector<droidCrypto::DeltaShare> computeLocalEuclidean(vector<vector<droidCrypto::DeltaShare>> database, vector<droidCrypto::DeltaShare> fingerprint, droidCrypto::AEnv& aEnv){
    //return testComputation(aEnv);
    vector<droidCrypto::DeltaShare> distances;
    vector<droidCrypto::DeltaShare> a;
    int M = database.size();
    int N = fingerprint.size();
    for (int i = 0; i < M ; ++i) {
        for (int j = 1; j < N; ++j) {
            droidCrypto::DeltaShare tmp = (database[i][j] - fingerprint[j]);
            a.push_back(tmp);
        }
        droidCrypto::DeltaShare tmp = aEnv.mult(a, a, i);
        distances.push_back(tmp);
    }
    return distances;
}

vector<droidCrypto::DeltaShare> computeOpenedEuclidean(droidCrypto::AEnv& aEnv){
    vector<droidCrypto::DeltaShare> distances;
    vector<droidCrypto::DeltaShare> scalarShares;
    std::chrono::duration<double> squareCompTime(0);
    std::chrono::duration<double> scalarProdTime(0);
    std::chrono::duration<double> euclideanTime(0);
    auto euclideanTimeStart = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < aEnv.M; ++i) {
        droidCrypto::DeltaShare scalarProduct = aEnv.mult(aEnv.fingerprint, aEnv.database[i], i);
        distances.push_back(scalarProduct);
    }
    scalarShares = aEnv.finalizeMultiplication(distances);
    //return scalarShares;
    distances.clear();
    auto timeScalarEnd = std::chrono::high_resolution_clock::now();
    scalarProdTime = scalarProdTime + (timeScalarEnd - euclideanTimeStart);

    for (int i = 0; i < aEnv.M; ++i) {
        auto timeSquareStart = std::chrono::high_resolution_clock::now();
        droidCrypto::DeltaShare sumSquaredValues = aEnv.dSquareSum[i] + aEnv.fingerprintSum;
        auto timeSquareEnd = std::chrono::high_resolution_clock::now();
        squareCompTime = squareCompTime + (timeSquareEnd - timeSquareStart);
        droidCrypto::DeltaShare tmp = scalarShares[i] + scalarShares[i];
        droidCrypto::DeltaShare distance = sumSquaredValues - tmp;
        //distances.push_back(sumSquaredValues);
        distances.push_back(distance);
    }
    auto euclideanTimeEnd = std::chrono::high_resolution_clock::now();
    euclideanTime =  (euclideanTimeEnd - euclideanTimeStart);
    droidCrypto::Log::v("MA", "Euclidean Communication time online phase: %f seconds", aEnv.communicationTime);
    droidCrypto::Log::v("MA", "scalarProdTime time online phase: %f seconds", scalarProdTime);
    droidCrypto::Log::v("MA", "square computation time online phase: %f seconds", squareCompTime);
    droidCrypto::Log::v("MA", "Euclidean time online phase: %f seconds", euclideanTime);

    return distances;
}

vector<droidCrypto::DeltaShare> testComputation(droidCrypto::AEnv& aEnv){
    vector<droidCrypto::DeltaShare> a;
    if(aEnv.isServer){
        droidCrypto::DeltaShare x(7, 2, 16);//=4
        droidCrypto::DeltaShare y(9, 15, 16);//=8
        droidCrypto::DeltaShare z = x + y;
        a.push_back(z);
    } else{
        droidCrypto::DeltaShare x(7, 1, 16);//=4
        droidCrypto::DeltaShare y(9, 2, 16);//=8
        droidCrypto::DeltaShare z = x + y;
        a.push_back(z);
    }
    return a;
}
