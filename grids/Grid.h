//
// Created by s4570405 on 21/06/2021.
//

#ifndef HIERARCHICALGRID_GRID_H
#define HIERARCHICALGRID_GRID_H

#include "../spatial/GeoPoint.h"

using namespace std;

class Grid {
    float *longitude_range{};    //[0] - min, [1] - max
    float *latitude_range{};
    float step{};    // the given step
    long numOfCellX_col{};    // longitude
    long numOfCellY_row{};    // latitude

public:
    void initializeSpace(float lng_min, float lng_max, float lat_min, float lat_max, float _step = 0.001);

    bool initializeSpace(Grid *gd, float _step);

    Grid() = default;

    bool gridCover(float lng, float lat);

    [[nodiscard]] bool is_invalid(long gridId) const;

    long getGridIdByGeopoint(float lng, float lat);

    long getGridIdByGeopoint(GeoPoint *point);

    void getSurrounding(long centerGridID, vector<long> &results) const;

    bool getSurrounding(long centerGridID, int iteration, vector<long> &results) const;

    virtual string printout();

    virtual void clearAll();

    float getLongitudeRange(int i) {
        return longitude_range[i];  //[0] - min, [1] - max
    }

    float getLatitudeRange(int i) {
        return latitude_range[i];
    }
};


#endif //HIERARCHICALGRID_GRID_H
