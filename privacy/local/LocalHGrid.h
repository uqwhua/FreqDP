//
// Created by Fengmei Jin on 14/11/21.
//

#ifndef HIERARCHICALGRID_LOCALHGRID_H
#define HIERARCHICALGRID_LOCALHGRID_H

#include "lGridCell.h"
#include "../HierarGrid.h"

class LocalHGrid: public HierarGrid{

    vector<unordered_map<int, vector<Point*>>> gid2pid2points;      // used for Local.deletion

    vector<lGridCell*> allGrids;    // all grid cells under this granularity, the index is the z-value for that grid

public:

    LocalHGrid(Grid *grid, int granularity, bool forDeletion);

    void connectWithLowerGrid(LocalHGrid *lowerHG);     // to connect between concept (spatial) parent/children

    void clearUselessInfo();

    void clearContent();

    void clearGrids() override;

    lGridCell *getGridCellByZID(int zid);

    // ---- for global insertion

    void insertSegment(int zid, Point *leftPoint, GeoPoint *lGP, Point *rightPoint = nullptr, GeoPoint *rGP = nullptr);     // maybe single-endpoint segment

    unsigned int splitSegments(int targetPid, GeoPoint *targetP, int expectIncrease, int zid, const vector<pair<lSegment *, int>>& segs,
                               map<int, map<int, vector<lSegment *>>> &level2zid2segments, long* updateTimeTotal, bool compressed = true);

    // ---- only performed on the finest grid

    void computeZgridForPoints(const Trip *trip, StaticGrid *sgrid);      // only used in finest granularity!!!

    int completeDeletePoint(int targetPid, int targetGid);

    int selectiveDeletePoint(int targetPid, int targetGid, int expectedDelete, StaticGrid *sgrid);

};


#endif //HIERARCHICALGRID_LOCALHGRID_H
