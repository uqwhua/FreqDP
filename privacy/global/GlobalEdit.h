//
// Created by s4570405 on 24/06/2021.
//

#ifndef HIERARCHICALGRID_GLOBALEDIT_H
#define HIERARCHICALGRID_GLOBALEDIT_H

#include "GlobalHGrid.h"
#include "../../linking/FrequencyVec.h"

using namespace std;

class GlobalEdit {

    static GlobalHGrid *constructFinestGrid(int granularity, StaticGrid *sgrid, const map<string, Trip *> &trajectories);

    static void buildGridStructure(StaticGrid *sgrid, vector<GlobalHGrid *> &multiGrids);

    static void buildGridContent(StaticGrid *sgrid, vector<GlobalHGrid *> &multiGrids, const string &uid, Trip *trip,
                                 vector<unordered_set<string>> &point2users);

    static long *singlePointInsertion(int targetPointId, int expectedIncrease, StaticGrid *sgrid, vector<GlobalHGrid *> &multiGrids,
                                      unordered_set<string> &unavailUsers, map<string, Trip *> &trajectories);

    static int singlePointDeletion(int targetPointId, int oriITF, int expectedDecrease,
                                   StaticGrid *sgrid, GlobalHGrid *finestGrid, map<string, Trip *> &trajectories);

public:
    static void modify(FrequencyVec *globalITF_ori, FrequencyVec *globalITF_pert, StaticGrid *sgrid,
                       map<string, Trip *> &trajectories, long *timecosts);
};


#endif //HIERARCHICALGRID_GLOBALEDIT_H
