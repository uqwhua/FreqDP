//
// Created by Fengmei Jin on 3/7/21.
//

#ifndef HIERARCHICALGRID_EDITOPERATION_H
#define HIERARCHICALGRID_EDITOPERATION_H

#include "Segment.h"

class EditOperation {

public:

    static float computeUtilityLossForPoint(StaticGrid *grid, Point *targetPoint) {
        return computeUtilityLossForPoint(grid, targetPoint->getPointId(), targetPoint->getPrevPoint(), targetPoint->getNextPoint());
    }

    // insert the target point between aheadP and nextP
    // or delete the target point from aheadP and nextP
    static float computeUtilityLossForPoint(StaticGrid *grid, int targetPointID, Point *aheadP, Point *nextP) {
        float cost;
        if (aheadP == nullptr || aheadP->isHeadPtr()) {
            if(nextP == nullptr || nextP->isTailPtr()) {
                cost = -1;            // invalid, should not happen, unless this trip has only one point
            }
            else {
                cost = grid->getPairwiseDist(targetPointID, nextP->getPointId());
            }
        }
        else if (nextP == nullptr || nextP->isTailPtr()) {
            cost = grid->getPairwiseDist(targetPointID, aheadP->getPointId());
        }
        else {
            int aheadPID = aheadP->getPointId(), nextPID = nextP->getPointId();
            if (aheadPID == targetPointID || nextPID == targetPointID) {
                cost = 0;
            }
            else {
                float d1 = grid->getPairwiseDist(targetPointID, aheadPID);
                float d2 = grid->getPairwiseDist(targetPointID, nextPID);
                float d3 = grid->getPairwiseDist(aheadPID, nextPID);
                cost = d1 + d2 - d3;
                cost = MAX(0, cost);    // if cost < 0, the targetPointID may locate within these two points, so just zero
            }
        }
        return cost;
    }

    template<class Component>
    static bool compareCost(pair<Component, float> &p1, pair<Component, float> &p2) {
        return p1.second < p2.second;   // sorted by the cost, ascending
    }

    static int computeAvailableInsertion(long leftTime, long rightTime){
        int num = 1;
        if (leftTime > 0 && rightTime > 0 && leftTime < rightTime) {
            num = ceil((float) (rightTime - leftTime) / SAMPLING_RATE);
            num = MAX(1, num);   // at least 1
            num = MIN(num, MAX_REPEAT_INSERTION);
        }
        return num;
    }

    static int computeAvailableInsertion(Point *prePoint, Point *nextPoint) {
        int num = 1;
        if (prePoint != nullptr && !prePoint->isHeadPtr() && nextPoint != nullptr && !nextPoint->isTailPtr()) {
            long timediff = nextPoint->getTimestamp() - prePoint->getTimestamp();    // unit: second
            num = ceil((float) timediff / SAMPLING_RATE);
            num = MAX(1, num);   // at least 1
            num = MIN(num, MAX_REPEAT_INSERTION);
        }
        return num;
    }
};

#endif //HIERARCHICALGRID_EDITOPERATION_H
