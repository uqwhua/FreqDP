//
// Created by Fengmei Jin on 14/11/21.
//

#ifndef HIERARCHICALGRID_GLOBALHGRID_H
#define HIERARCHICALGRID_GLOBALHGRID_H

#include "gGridCell.h"
#include "../HierarGrid.h"

class GlobalHGrid : public HierarGrid{

    // only exists in FINEST_GRANULARITY
    vector<unordered_map<int, map<string, vector<Point*>>>> gid2pid2uid2points;     // used for Global.deletion

    vector<gGridCell*> allGrids;    // for all grids

public:

    GlobalHGrid(Grid *_grid, int _granularity, bool forDeletion = false);

    gGridCell *getGridCellByZID(int zid);

    void connectWithLowerGrid(GlobalHGrid *lowerHG);

    // ---- only on the finest grid

    void computeZgridForPoints(const string &uid, const Trip *trip, StaticGrid *sgrid);

    void selectTopUsersForGlobalPointDeletion(int targetGridId, int targetPid, int expectDecrease, StaticGrid *sgrid, vector<pair<string, float>> &user2costs);

    map<string, int> completeDeletePointFromGrid(int targetPointId, int targetGridId, set<string> &selectedUsers);

    void clearUselessInfo();

    void clearGrids() override;

    void insertSegment(const string &uid, int zid, Point *leftPoint, GeoPoint *lGP, Point *rightPoint = nullptr, GeoPoint *rGP = nullptr);
};


#endif //HIERARCHICALGRID_GLOBALHGRID_H
