//
// Created by s4570405 on 23/06/2021.
//

#ifndef HIERARCHICALGRID_FUNCTIONS_H
#define HIERARCHICALGRID_FUNCTIONS_H


#include "../spatial/Trip.h"
#include "Signature.h"
#include "FrequencyVec.h"

class Functions {

    static float constructSignatures(const map<string, unsigned int>& user2trajLen, const map<string, FrequencyVec *>& freqVectors,
                                     float* point2IDF, map<string, Signature *>& signatures){

        float avgDimension = 0;
        for (const auto& fv: freqVectors) {
            string uid = fv.first;
            auto *sig = new Signature;
            fv.second->computeTFIDF(user2trajLen.at(uid), point2IDF, sig); // inside, from frequency to TF, then TF*IDF
            signatures[uid] = sig;
            avgDimension += (float) sig->getLength();
        }
        return avgDimension / (float) signatures.size();
    }

public:
    static float construction(const map<string, Trip *>& trajectories, map<string, Signature *>& signatures,
                               map<string, FrequencyVec *>& freqVectors, FrequencyVec *globalPointFreq = nullptr) {
        bool needClear = (globalPointFreq == nullptr);
        if(globalPointFreq == nullptr) {
            globalPointFreq = new FrequencyVec;
        }
        signatures.clear();
        freqVectors.clear();

        // count the frequency for each participant
        map<string, unsigned int> user2trajLen;
        for (const auto& pair: trajectories) {
            string userID = pair.first;
            Trip* traj = pair.second;
            unsigned int length = traj->getLength();
            user2trajLen[userID] = length;

            // frequency (point count)
            auto *fv = new FrequencyVec(traj);
            freqVectors[userID] = fv;

            // prepare for ITF computation
            globalPointFreq->mergeForITF(fv);
        }

        float *idfVec = globalPointFreq->computeIDF(trajectories.size());

        float avgDimension = constructSignatures(user2trajLen, freqVectors, idfVec, signatures);

        if (signatures.size() != trajectories.size()) {
            printf("[ALERT] the number of constructed signatures %lu is inconsistent with trajectory number %lu\n",
                   signatures.size(), trajectories.size());
        }

        user2trajLen.clear();
        delete idfVec;
        idfVec = nullptr;

        if(needClear) {
            globalPointFreq->clearArray();
            delete globalPointFreq;
            globalPointFreq = nullptr;
        }

        return avgDimension;
    }

    static unordered_set<int> extractTopSignaturePoints(const map<string, Signature *>& signatures, int cutoff){
        unordered_set<int> sigpoints, all_sigpoints;
        for (auto pair: signatures) {
            string userID = pair.first;
            pair.second->reduceTopM(cutoff, sigpoints);

            all_sigpoints.insert(sigpoints.begin(), sigpoints.end());
            sigpoints.clear();
        }
        return all_sigpoints;
    }
};


#endif //HIERARCHICALGRID_FUNCTIONS_H
