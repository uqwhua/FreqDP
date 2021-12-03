//
// Created by s4570405 on 23/06/2021.
//

#include "FrequencyVec.h"

// mainly for global ITF distribution
FrequencyVec::FrequencyVec(const int& len) {
    arrayLen = len;
    freqVec = new int[len] {0};
}

FrequencyVec::FrequencyVec(Trip *trip, const int &len) {
    arrayLen = len;
    freqVec = new int[len] {0};
    auto p = trip->getFirstPoint();
    while (p != nullptr && !p->isTailPtr()) {
        freqVec[p->getPointId()]++;
        p = p->getNextPoint();
    }
}

FrequencyVec::FrequencyVec(FrequencyVec *pVec) {
    arrayLen = pVec->arrayLen;
    freqVec = new int[arrayLen];
    for(int i = 0; i < arrayLen; i++){
        freqVec[i] = pVec->freqVec[i];
    }
}

void FrequencyVec::mergeForITF(FrequencyVec *fv) {
    for(int id = 0; id < arrayLen; id++){
        if(fv->freqVec[id] > 0){
            this->freqVec[id] ++;
        }
    }
}

float* FrequencyVec::computeIDF(unsigned int total) {
    auto* idfVec = new float[arrayLen] {0};

    for (int id = 0; id < arrayLen; id++){
        if(freqVec[id] > 0){
            idfVec[id] = (float) log((total + 1) * 1.0 / freqVec[id]);
        }
    }
    return idfVec;
}

void FrequencyVec::computeTFIDF(unsigned int length, const float* point2IDF, Signature *signature) {
    float sqrtSum = 0;
    for(int id = 0; id < arrayLen; id++){
        float tf = (float) freqVec[id] / (float) length;     // from frequency to TF
        float tfidf = tf * point2IDF[id];
        if(tfidf > 0) {
            signature->insert(id, tfidf);
            sqrtSum += tfidf * tfidf;
        }
    }
    signature->normalize(sqrt(sqrtSum));
}

void FrequencyVec::insertPair(int key, int value) {
    freqVec[key] = value;   // insert or update
}

int FrequencyVec::getValueByKey(int pid) {
    if(pid < arrayLen){
        return freqVec[pid];
    }
    return -1;
}

void FrequencyVec::copyValue(FrequencyVec *pVec) {
    if(pVec->freqVec != nullptr) {
        for (int i = 0; i < arrayLen; i++) {
            this->freqVec[i] = pVec->freqVec[i];
        }
    }
}

void FrequencyVec::updateValue(int pid, int newValue) {
    if(pid < arrayLen){
        freqVec[pid] = newValue;
    }
}

int FrequencyVec::getArraryLen() const {
    return arrayLen;
}

void FrequencyVec::clearArray() {
    delete freqVec;
    freqVec = nullptr;
}

