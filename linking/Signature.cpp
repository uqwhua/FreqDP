//
// Created by s4570405 on 23/06/2021.
//

#include "Signature.h"
#include "../privacy/NoiseGenerator.h"

void Signature::normalize(float sqrtSum) {
    for(auto &ptr: signatureVec){
        ptr.second /= sqrtSum;
    }
}

void Signature::insert(int pid, float weight) {
    signatureVec[pid] = weight;
}

bool comparePair(pair<int, float>& a, pair<int, float>& b){
    return a.second > b.second; // descending order
}

vector<pair<int, float>> Signature::sortMapByValue() {
    vector<pair<int, float>> sortedVec;
    sortedVec.reserve(signatureVec.size());
    for(auto pair: signatureVec){
        sortedVec.emplace_back(make_pair(pair.first, pair.second));
    }
    sort(sortedVec.begin(), sortedVec.end(), comparePair);
    return sortedVec;
}

void Signature::reduceTopM(int cutoff, unordered_set<int> &pointset) {

    if(cutoff <= 0 || cutoff >= signatureVec.size()){
        for(auto pair: signatureVec){
            pointset.insert(pair.first);
        }
    }
    else {
        vector<pair<int, float>> sortedVec = sortMapByValue();

        int c = 0;
        auto itr = sortedVec.begin();
        while (c++ < cutoff && itr != sortedVec.end()) {
            pointset.insert(itr->first);
            itr++;
        }
    }
}

unsigned int Signature::getLength() const {
    return signatureVec.size();
}

void Signature::clearMap() {
    signatureVec.clear();
}

// Range: [start, end)
vector<int> Signature::getPointListWithinRange(int startPos, int endPos) {
    vector<int> pointList;
    vector<pair<int, float>> sortedVec = sortMapByValue();

    for(unsigned int i = startPos, total = sortedVec.size(); i < endPos && i < total; i++) {
        pointList.emplace_back(sortedVec.at(i).first);
    }
    return pointList;
}

unordered_set<int> Signature::randomSelectPoints(int expectedNum, const vector<int>& unavailablePoints) {
    unordered_set<int> unavailableSet;
    unavailableSet.insert(unavailablePoints.begin(), unavailablePoints.end());

    unordered_set<int> selectedPoints;
    int remainNum = (int) signatureVec.size();
    auto itr = signatureVec.begin();
    while (itr != signatureVec.end() && expectedNum > 0) {
        int randomValue = NoiseGenerator::intFromUniformDistribution(0, remainNum);
        if(randomValue < expectedNum) {
            int pid = itr->first;
            auto itr2 = unavailableSet.find(pid);
            if(itr2 == unavailableSet.end()) {
                selectedPoints.insert(pid);
                expectedNum--;
            }
            else {
                unavailableSet.erase(itr2);
            }
        }
        remainNum--;
        itr++;
    }
    unavailableSet.clear();
    return selectedPoints;
}

vector<int> Signature::selectPointsFromGivenSet(const unordered_set<int> &candidatePoints, int expectedNum, bool limitNum) {
    vector<int> pointList;
    vector<pair<int, float>> sorted = sortMapByValue();

    // in stage-1, we've selected "expectedNum" points already
    // so this round, the selection starts from "expectedNum"
    for(int i = expectedNum; i < sorted.size(); i++) {
        int pid = sorted.at(i).first;
        if(candidatePoints.find(pid) != candidatePoints.end()) {    // exist in the candidate set
            pointList.emplace_back(pid);
        }
        if(limitNum && pointList.size() >= expectedNum){
            break;
        }
    }
    return pointList;
}
