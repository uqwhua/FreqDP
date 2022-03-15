//
// Created by s4570405 on 24/06/2021.
//

#ifndef HIERARCHICALGRID_PERTURBDISTRIBUTION_H
#define HIERARCHICALGRID_PERTURBDISTRIBUTION_H

#include "../linking/Signature.h"
#include "../linking/FrequencyVec.h"
#include "../commonheader.h"
#include "NoiseGenerator.h"

class PerturbDistribution {

    static int postProcessing(int freq_ori, float noise) {
        float freq_pert = (float) freq_ori + noise;
        freq_pert = round(freq_pert);
        freq_pert = MAX(freq_pert, 0);  // round negative value to zero
        return (int) freq_pert;
    }

public:
    // for frequency perturbation
    static void perturbFrequency(const map<string, FrequencyVec *>& freqVec_ori, const map<string, vector<int>>& targetSigPoints,
                                 int cutoff, float epsilon, map<string, FrequencyVec *> &freqVec_pert){
        for (const auto& fpair: freqVec_ori) {
            string userID = fpair.first;
            FrequencyVec* freq_ori = fpair.second;
            auto itr = targetSigPoints.find(userID);
            if(itr != targetSigPoints.end()) {
                vector<int> targetPoints = itr->second;

                unsigned int pointNum = targetPoints.size();

                auto *freq_pert = new FrequencyVec(freq_ori);

                // Stage-1: top1 - topM
                float noiseLowerBound = INFINITY;
                float avgAddedNoise1 = 0;
                int i;
                for (i = 0; i < cutoff && i < pointNum; i++) {
                    int pid = targetPoints.at(i);
                    int freqValue_ori = freq_ori->getValueByKey(pid);
                    float noise = NoiseGenerator::fromLaplaceDistribution(epsilon, 1, -1.0f * (float) freqValue_ori);     // currently, fixed

                    int freqValue_pert = postProcessing(freqValue_ori, noise);
                    freq_pert->updateValue(pid, freqValue_pert);

                    float addedNoise = (float) freqValue_pert - (float) freqValue_ori;
                    avgAddedNoise1 += addedNoise;

                    noiseLowerBound = MIN(noiseLowerBound, addedNoise);     // should be a negative value
                }

                avgAddedNoise1 /= (float) i;

                // Stage-2: top-(M+1) - top-2M
                float noiseUpperBound = abs(noiseLowerBound);
                for (i = cutoff; i < pointNum; i++) {
                    int pid = targetPoints.at(i);
                    int freqValue_ori = freq_ori->getValueByKey(pid);
                    float noise = NoiseGenerator::fromLaplaceDistribution(epsilon, 1, -1 * avgAddedNoise1);

                    noise = MIN(noise, noiseUpperBound);
                    int freqValue_pert = postProcessing(freqValue_ori, noise);
                    freq_pert->updateValue(pid, freqValue_pert);
                }

                freqVec_pert[userID] = freq_pert;
            }
        }
    }

    // for global ITF perturbation
    static void perturbITF(const unordered_set<int>& topmSigPoints, int total_obj, float epsilon, FrequencyVec *globalPointFreq_pert) {
        float mean = 0;
        for (int pid: topmSigPoints) {
            auto itf_ori = (float) globalPointFreq_pert->getValueByKey(pid);
            float itf_pert = itf_ori + NoiseGenerator::fromLaplaceDistribution(epsilon, 1, mean);
            itf_pert = round(itf_pert);
            itf_pert = MAX(itf_pert, 0);     // post-processing: round all negative values to zero
            itf_pert = MIN(itf_pert, total_obj);
            globalPointFreq_pert->insertPair(pid, (int) itf_pert);
        }
    }

};


#endif //HIERARCHICALGRID_PERTURBDISTRIBUTION_H
