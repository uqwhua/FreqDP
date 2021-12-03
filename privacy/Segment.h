//
// Created by Fengmei Jin on 3/11/21.
//

#ifndef HIERARCHICALGRID_SEGMENT_H
#define HIERARCHICALGRID_SEGMENT_H

#include <utility>
#include "../spatial/Point.h"
#include "../grids/StaticGrid.h"

class Segment {

protected:
    GeoPoint *leftPoint;
    GeoPoint *rightPoint;   // might be null for the first and last segment

public:

    Segment(GeoPoint *&leftP, GeoPoint *&rightP): leftPoint(leftP), rightPoint(rightP) {}

    virtual void setNull();

    virtual bool empty() = 0;

    virtual void merge(Segment *seg) = 0;

    [[nodiscard]] virtual unsigned int getPointPairNum() const = 0;

    // ************ below functions only about spatial (GeoPoint, two endpoints), can be shared by LOCAL and GLOBAL

    bool equal(Segment *seg);

    GeoPoint *getPoint(bool left = true);

    [[nodiscard]] int getStartPointId();    // aka left point id (in most cases)

    [[nodiscard]] int getPointId(bool left = true) const;

    [[nodiscard]] float computeDistance(int targetPid, const GeoPoint *targetP, StaticGrid *sgrid, float distToLeft = -1) const;
};

#endif //HIERARCHICALGRID_SEGMENT_H
