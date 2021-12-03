//
// Created by s4570405 on 23/06/2021.
//

#ifndef HIERARCHICALGRID_FREQUENCYVEC_H
#define HIERARCHICALGRID_FREQUENCYVEC_H

#include <cmath>
#include <map>
#include <utility>
#include <algorithm>
#include "Signature.h"
#include "../spatial/Trip.h"
#include "../grids/StaticGrid.h"

using namespace std;

class FrequencyVec{
    int* freqVec;
    int arrayLen;

public:
    explicit FrequencyVec(Trip *trip, const int& len = ARRAY_LENGTH);

    explicit FrequencyVec(const int& len = ARRAY_LENGTH);

    explicit FrequencyVec(FrequencyVec *pVec);

    void mergeForITF(FrequencyVec *fv);

    float* computeIDF(unsigned int total);

    void computeTFIDF(unsigned int length, const float* idfVec, Signature *signature);

    void insertPair(int key, int value);

    int getValueByKey(int pid);

    void copyValue(FrequencyVec *pVec);

    void updateValue(int pid, int newValue);

    [[nodiscard]] int getArraryLen() const;

    void clearArray();
};


#endif //HIERARCHICALGRID_FREQUENCYVEC_H
