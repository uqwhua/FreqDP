//
// Created by Fengmei Jin on 3/11/21.
//

#include "Segment.h"

int Segment::getStartPointId() {
    return leftPoint->getPointId();
}

int Segment::getPointId(bool left) const {
    return left ? leftPoint->getPointId() : rightPoint->getPointId();
}

GeoPoint *Segment::getPoint(bool left) {
    return left ? leftPoint : rightPoint;
}

float length_squared(GeoPoint *p1, GeoPoint *p2) {
    float l = 0;
    if (p1 != nullptr && p2 != nullptr) {
        float x = p1->getLongitude() - p2->getLongitude();
        float y = p1->getLatitude() - p2->getLatitude();
        return x * x + y * y;
    }
    return l;
}

float Segment::computeDistance(int targetPid, const GeoPoint *targetP, StaticGrid *sgrid, float distToLeft) const {
    if(rightPoint == nullptr) {
        return distToLeft;
    }
    int lid = leftPoint->getPointId(), rid = rightPoint->getPointId();

    // Return minimum distance between line segment vw and targetP p
    const float distSq = length_squared(leftPoint, rightPoint);  // i.e. |w-v|^2 -  avoid a sqrt
    if (distSq == 0)
        return distToLeft;   // left == right case

    // NOTE: v -- lP, w -- rP
    // Consider the line extending the segment, parameterized as v + t (w - v).
    // We find projection of targetP p onto the line.
    // It falls where t = [(p-v) dot (w-v)] / |w-v|^2
    // We clamp t from [0,1] to handle points outside the segment vw.
    GeoPoint diff = *rightPoint - *leftPoint;
    const float t = (*targetP - *leftPoint).computeDotProduct(diff) / distSq;
    if (t < 0.0) {
        // beyond the v end of the segment
        return distToLeft < 0 ? sgrid->getPairwiseDist(lid, targetPid) : distToLeft;
    }
    else if (t > 1.0) {
        // beyond the w end of the segment
        return sgrid->getPairwiseDist(targetPid, rid);
    }

    GeoPoint projection = *leftPoint + diff * t;  // Projection falls on the segment
    return targetP->computeDistance(&projection);
}

// only compare GeoPoint
bool Segment::equal(Segment *seg) {
    return (seg->leftPoint == leftPoint && seg->rightPoint == rightPoint)
           ||  (seg->leftPoint == rightPoint && seg->rightPoint == rightPoint);
}

void Segment::setNull() {
    if(leftPoint != nullptr) {
        leftPoint = nullptr;
    }
    if(rightPoint != nullptr) {
        rightPoint = nullptr;
    }
}
