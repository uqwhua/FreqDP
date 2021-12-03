//
// Created by Fengmei JIN on 27/8/21.
//

#ifndef HIERARCHICALGRID_GLOBALMECHANISM_H
#define HIERARCHICALGRID_GLOBALMECHANISM_H

#include "../../io/LogFile.h"
#include "../../linking/Functions.h"
#include "../PerturbDistribution.h"
#include "GlobalEdit.h"

class GlobalMechanism {

public:

    static void perform(int data_total, float eps, int cutoff, LogFile *logger, StaticGrid* sgrid, map<string, Trip *> &trajectories_cur,
                         FrequencyVec *globalITF_cur = nullptr, unordered_set<int> sigPoints = unordered_set<int>()) {

        string msg = "---------------------------------\n";
        msg.append("[INFO] Globally perturbing ITF and modifying trajectories ...");
        logger->addContent(msg);

        // if the original values are not given
        // construct signatures based on current trajectories
        bool needClear = false;
        map<string, Signature*> signatures_cur;
        map<string, FrequencyVec*> freqVec_cur;
        if(globalITF_cur == nullptr || sigPoints.empty()){
            needClear = true;
            globalITF_cur = new FrequencyVec;
            Functions::construction(trajectories_cur, signatures_cur, freqVec_cur, globalITF_cur);
            sigPoints = Functions::extractTopSignaturePoints(signatures_cur, cutoff);
        }

        // start global alteration
        long* timeCosts = new long[8]{};   // record detailed time costs

        long startTimer = Utils::millisecond();
        auto *globalITF_pert = new FrequencyVec;
        globalITF_pert->copyValue(globalITF_cur);   // now, they are same
        PerturbDistribution::perturbITF(sigPoints, data_total, eps, globalITF_pert);
        timeCosts[0] = Utils::millisecond() - startTimer;   // perturbation time

        startTimer = Utils::millisecond();  // reset
        GlobalEdit::modify(globalITF_cur, globalITF_pert, sgrid, trajectories_cur, timeCosts);
        timeCosts[1] = Utils::millisecond() - startTimer;

        msg.append("\n[TIME COST] during Global Mechanism\n");
        msg.append("\t ITF Perturbation = " + to_string(timeCosts[0]) + " ms\n");
        msg.append("\t Trj Modification = " + to_string(timeCosts[1]) + " ms\n");
        msg.append("\t\t -> Build HGrids = " + to_string(timeCosts[7]) + " ms\n");
        msg.append("\t\t -> ITF-Decrease = " + to_string(timeCosts[2]) + " ms\n");
        msg.append("\t\t -> ITF-Increase = " + to_string(timeCosts[3]) + " ms\n");
        msg.append("\t\t\t --> Search Top Segments = " + to_string(timeCosts[4]) + " ms\n");
        msg.append("\t\t\t --> PointInsertion Time = " + to_string(timeCosts[5]) + " ms\n");
        msg.append("\t\t\t --> H-Grids Update Time = " + to_string(timeCosts[6]) + " ms\n");

        logger->addContent(msg);

        globalITF_pert->clearArray();
        delete globalITF_pert;
        globalITF_pert = nullptr;

        delete [] timeCosts;
        timeCosts = nullptr;

        if(needClear) {
            Utils::mapClear_Sig(signatures_cur);
            Utils::mapClear_Freq(freqVec_cur);

            globalITF_cur->clearArray();
            delete globalITF_cur;
            globalITF_cur = nullptr;

            sigPoints.clear();
        }
    }
};

#endif //HIERARCHICALGRID_GLOBALMECHANISM_H
