//
// Created by Fengmei JIN on 27/8/21.
//

#ifndef HIERARCHICALGRID_LOCALMECHANISM_H
#define HIERARCHICALGRID_LOCALMECHANISM_H

#include "../../linking/Functions.h"
#include "../../io/LogFile.h"
#include "../PerturbDistribution.h"
#include "LocalEdit.h"

class LocalMechanism {

public:

    // used in Global --> Local
    static void perform(LogFile *logger, int cutoff, float eps, StaticGrid* sgrid, map<string, Trip *> &trajectories_cur) {

        // if the original values are not given
        // we need extract signatures from the given trajectories
        map<string, Signature *> signatures_cur;
        map<string, FrequencyVec *> freqVec_cur;

        Functions::construction(trajectories_cur, signatures_cur, freqVec_cur);
        unordered_set<int> sigPoints = Functions::extractTopSignaturePoints(signatures_cur, cutoff);
        map<string, vector<int>> targetTopPoints = getTargetSigPoints(signatures_cur, cutoff, sigPoints);

        perform(logger, cutoff, eps, sgrid, trajectories_cur, signatures_cur, freqVec_cur, targetTopPoints);

        targetTopPoints.clear();
        Utils::mapClear_Freq(freqVec_cur);
        Utils::mapClear_Sig(signatures_cur);
    }

    static void perform(LogFile *logger, int cutoff, float eps, StaticGrid *static_grid, map<string, Trip *> &trajectories_cur,
                        const map<string, Signature *> &signatures_cur, const map<string, FrequencyVec *> &freqVec_cur,
                        const map<string, vector<int>> &targetTopPoints) {

        string msg = "---------------------------------\n";
        msg.append("[INFO] Locally perturbing frequency and modifying trajectories ...");
        logger->addContent(msg);

        // start perturbation and alteration
        map<string, FrequencyVec *> freqVec_pert;
        long* timeCosts = new long[8]{};    // record detailed time costs

        long startTimer = Utils::millisecond();
        PerturbDistribution::perturbFrequency(freqVec_cur, targetTopPoints, cutoff, eps, freqVec_pert);
        timeCosts[0] = Utils::millisecond() - startTimer;

        startTimer = Utils::millisecond();
        LocalEdit::modify(freqVec_cur, freqVec_pert, static_grid, trajectories_cur, timeCosts);
        timeCosts[1] = Utils::millisecond() - startTimer;

        msg.append("\n[TIME COST] during Local Mechanism\n");
        msg.append("\t Freq Perturbation = " + to_string(timeCosts[0]) + " ms\n");
        msg.append("\t Traj Modification = " + to_string(timeCosts[1]) + " ms\n");  // total
        msg.append("\t\t -> Build Grids = " + to_string(timeCosts[7]) + " ms\n");
        msg.append("\t\t -> PF-Decrease = " + to_string(timeCosts[2]) + " ms\n");
        msg.append("\t\t -> PF-Increase = " + to_string(timeCosts[3]) + " ms\n");
        msg.append("\t\t\t --> Search Top Segments = " + to_string(timeCosts[4]) + " ms\n");
        msg.append("\t\t\t --> PointInsertion Time = " + to_string(timeCosts[5]) + " ms\n");
        msg.append("\t\t\t --> H-Grids Update Time = " + to_string(timeCosts[6]) + " ms\n");

        logger->addContent(msg);

        Utils::mapClear_Freq(freqVec_pert);
        delete[] timeCosts;
        timeCosts = nullptr;
    }

    static map<string, vector<int>> getTargetSigPoints(const map<string, Signature *> &signatures, int cutoff,
                                                       const unordered_set<int> &sigpoints,
                                                       bool limitOtherSigNum = true) {
        map<string, vector<int>> user2points;
        for (const auto &eachUser: signatures) {
            Signature *sig = eachUser.second;

            // in Stage-1: top-1 to top-m
            vector<int> chosenPoints = sig->getPointListWithinRange(0, cutoff);

            // in Stage-2, preferSig
            vector<int> otherSigPoints = sig->selectPointsFromGivenSet(sigpoints, cutoff, limitOtherSigNum);
            if (!otherSigPoints.empty()) {
                chosenPoints.insert(chosenPoints.end(), otherSigPoints.begin(), otherSigPoints.end());
            }

            int notEnough = cutoff - (int) otherSigPoints.size();
            if (notEnough > 0) {
                unordered_set<int> randomPoints = sig->randomSelectPoints(notEnough, chosenPoints);
                chosenPoints.insert(chosenPoints.end(), randomPoints.begin(), randomPoints.end());
            }

            if (!chosenPoints.empty()) {
                user2points[eachUser.first] = chosenPoints;
            }
        }
        return user2points;
    }
};

#endif //HIERARCHICALGRID_LOCALMECHANISM_H
