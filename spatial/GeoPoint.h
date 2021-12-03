//
// Created by s4570405 on 21/06/2021.
//

#ifndef HIERARCHICALGRID_GEOPOINT_H
#define HIERARCHICALGRID_GEOPOINT_H

#include "../commonheader.h"

using namespace std;

class GeoPoint {
    int pid;
    float *coordinates;

public:
    GeoPoint(int id, float lng, float lat) : pid(id), coordinates(new float[2]{lng, lat}) {};

    [[nodiscard]] string toString() const;

    [[nodiscard]] int getPointId() const;

    [[nodiscard]] float getLongitude() const;

    [[nodiscard]] float getLatitude() const;

    [[nodiscard]] float computeDistance(float lng, float lat) const;

    float computeDistance(const GeoPoint *point) const;

    static float computeDistanceByCoors(float lng1, float lat1, float lng2, float lat2);   // in general use

    void clearCoors();

    float computeDotProduct(GeoPoint point);

    GeoPoint operator-(const GeoPoint &p) const {
        return {-1, this->coordinates[0] - p.coordinates[0], this->coordinates[1] - p.coordinates[1]};
    }

    GeoPoint operator+(const GeoPoint &p) const {
        return {-1, this->coordinates[0] + p.coordinates[0], this->coordinates[1] + p.coordinates[1]};
    }

    GeoPoint operator*(const float value) const {
        return {-1, value * this->coordinates[0], value * this->coordinates[1]};
    }
};


#endif //HIERARCHICALGRID_GEOPOINT_H
