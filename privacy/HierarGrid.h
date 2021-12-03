//
// Created by Fengmei Jin on 28/10/21.
//

#ifndef HIERARCHICALGRID_HIERARGRID_H
#define HIERARCHICALGRID_HIERARGRID_H


#include "GridCell.h"
#include "../spatial/Trip.h"

class HierarGrid {

protected:
    float* longitude_range;    //[0] - min, [1] - max
    float* latitude_range;
    float* step;       // [0] - longitude, [1] - latitude

    int granularity;    // 512*512, 256*256, 128*128, 64*64, 32*32, 16*16, 8*8, 4*4, ... depends on the longest segment

    bool gridCover(const GeoPoint *point);     // only related to space

    static bool isInvalidID(int gid, int _granularity);

    static int getZvalueByRowCol(int row, int col);

    static int* getRowColFromZvalue(int zvalue);

public:

    static int *upgradeZvalue(const int *lowerZ, int gap = 1);      // go to coarser granularity by popping a bit iteratively

    HierarGrid(Grid *_grid, int _granularity);

    [[nodiscard]] int getGranularity() const;

    virtual void clearGrids();

    int *locateGeoPoint(const GeoPoint *point, int _granularity = -1);    // return the zvalue of the grid where the given point locates

    int *findBestFitGrid(Segment *seg, const int *lz, const int *rz);

//    void aggregateChildInfo(unordered_map<string, vector<GridCell*>>& prefix2pointers);

    struct QNode{
        GridCell* _gc;
        float _minDist;    // a lower bound

        QNode(GridCell *gcell, float mdist): _gc(gcell), _minDist(mdist) {};
    };

    struct QNodeComp {
        bool operator() (QNode &n1, QNode &n2) {
//            if (n1._minDist == n2._minDist)
//                return n1._gc->getSegmentNum() < n2._gc->getSegmentNum();     // the grid with more segments are more promising
            return n1._minDist > n2._minDist;     // the node with smaller _minDist should be considered earlier
        }
    };

    typedef priority_queue<QNode, vector<QNode>, QNodeComp> GridQueue;
};


#endif //HIERARCHICALGRID_HIERARGRID_H
