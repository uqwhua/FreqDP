//
// Created by s4570405 on 28/06/2021.
//

#ifndef HIERARCHICALGRID_LOCALEDIT_H
#define HIERARCHICALGRID_LOCALEDIT_H


#include "LocalHGrid.h"
#include "../../linking/FrequencyVec.h"

class LocalEdit {

    static void buildGridStructure(StaticGrid *sgrid, vector<LocalHGrid *> &multiGrids);

    static vector<unordered_set<int>> buildGridContent(StaticGrid *sgrid, vector<LocalHGrid *> &multiGrids, const Trip *trip);

    static void compressGridCells(vector<LocalHGrid *> &multiGrids, vector<unordered_set<int>> &validGridCells);

    static long *increaseOccurrence(int targetPointId, int expectedIncrease, const string &userID,
                                    StaticGrid *sgrid, vector<LocalHGrid *> &multiGrids, Trip *trajectory, bool compress = true);

    static int reduceOccurrence(int targetPointId, int oriFreqValue, int expectedDelete, const string &userID,
                                StaticGrid *sgrid, LocalHGrid *finestGrid, Trip *trajectory);

public:
    static void modify(const map<string, FrequencyVec *> &allFreqVec_ori, const map<string, FrequencyVec *> &allFreqVec_pert,
                       StaticGrid *sgrid, map<string, Trip *> &trajectories, long *timeCosts);
};


#endif //HIERARCHICALGRID_LOCALEDIT_H
