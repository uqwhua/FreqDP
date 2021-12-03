//
// Created by s4570405 on 23/06/2021.
//

#ifndef HIERARCHICALGRID_SIGNATURE_H
#define HIERARCHICALGRID_SIGNATURE_H

#include <map>
#include <vector>
#include <utility>
#include <unordered_set>
#include <string>
#include <algorithm>

using namespace std;

class Signature{

    map<int, float> signatureVec;  // pair of (pointID, its TF-IDF weight)

public:

    void normalize(float sqrtSum);

    void insert(int pid, float weight);

    vector<pair<int, float>> sortMapByValue();

    void reduceTopM(int cutoff, unordered_set<int> &pointset);

    [[nodiscard]] unsigned int getLength() const;

    void clearMap();

    vector<int> getPointListWithinRange(int startPos, int endPos);

    unordered_set<int> randomSelectPoints(int expectedNum, const vector<int>& unavailablePoints);

    vector<int> selectPointsFromGivenSet(const unordered_set<int> &candidatePoints, int expectedNum, bool limitNum = true);
};


#endif //HIERARCHICALGRID_SIGNATURE_H
