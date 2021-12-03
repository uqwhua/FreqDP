//
// Created by Fengmei Jin on 28/10/21.
//

#ifndef HIERARCHICALGRID_GRIDCELL_H
#define HIERARCHICALGRID_GRIDCELL_H

#include "../commonheader.h"
#include "EditOperation.h"
#include <set>

// a single grid cell
class GridCell {
    float *longitude_range{};    //[0] - min, [1] - max
    float *latitude_range{};

    string zStr;

    // two types of pointers
    GridCell *structuralParent{};     // initially nullptr
    vector<GridCell *> structuralChildren;     // four children in space (like quad-tree)

    // if this grid cell is invalid, then thest two pointers below are null as well
    GridCell *validParent{};    // the closest valid ancestor (the difference may be more than one level)
    vector<GridCell *> validChildren;    // the valid children

protected:
    int granularity;
    int zvalue;    // the index as well

public:
    static int transferBaseFourToTen(string baseFourStr);

    static string transferBaseTenToFour(int value, int radix = 4);

    // constructor
    GridCell(int z, int l) : zvalue(z), zStr(transferBaseTenToFour(z)), granularity(l) {};

    // spatial related functions
    void setRange(float lat_min, float lat_max, float lng_min, float lng_max);

    float computeMinDist(const GeoPoint *point);     // the minimum distance between the point and this grid cell

    bool isCover(const GeoPoint *point);

    // pointer related functions
    void setParent(GridCell *parent, bool valid = false);

    GridCell *getParent(bool valid = false);

    vector<GridCell *> getChildren(bool validChild = false);

    void addChild(GridCell *ptr, bool validChild = false);   // update

    int *checkStructuralPointers();

    // general functions
    [[nodiscard]] int getGranularity() const;

    [[nodiscard]] int getID() const;

    string getBaseFourStr();

    virtual void clearAll();

    virtual void clearSegments() = 0;

    [[nodiscard]] virtual bool isValid() const = 0;

    [[nodiscard]] virtual unsigned int getSegmentNum(bool trajSeg) const = 0;

    // only for valid pointers
    void unlinkSelf();

    void removeChild(GridCell *child);

    void addChildren(vector<GridCell *> _children);

    void activateSelf();    // this grid cell become valid, need to make sure the pointers

    void clearValidPointers();
};


#endif //HIERARCHICALGRID_GRIDCELL_H
