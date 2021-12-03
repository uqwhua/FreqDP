//
// Created by Fengmei Jin on 26/11/21.
//

#include "lSegment.h"
#include "../EditOperation.h"

bool lSegment::empty() {
    return trajPointPairs.empty();
}

void lSegment::merge(Segment *seg) {
    auto *lSeg = dynamic_cast<lSegment *>(seg);
    for(auto pp: lSeg->trajPointPairs) {
        this->trajPointPairs.emplace_back(pp);
    }
}

int lSegment::computeAvailableInsertion() const {
    int total = 0;
    for(auto timePair: trajPointPairs) {
        total += EditOperation::computeAvailableInsertion(timePair.first, timePair.second);
    }
    return total;
}

void lSegment::insertNewPair(pair<Point *, Point *> newPair) {
    trajPointPairs.emplace_back(newPair);
}

unsigned int lSegment::getPointPairNum() const {
    return trajPointPairs.size();
}

/**
 * one original segment will be split into more than
 * two components including leftSeg, rightSeg and samePointSeg (if possible)
 */
tuple<lSegment *, lSegment *, lSegment *> lSegment::split_insert(int targetPid, GeoPoint *targetP, int avail_expect, int expectIncrease) {

    lSegment *leftSeg = nullptr, *rightSeg = nullptr, *samePointSeg = nullptr;

    int total = 0;
    auto pointPair = trajPointPairs.begin();
    while (pointPair != trajPointPairs.end() && total < expectIncrease) {
        auto p1 = pointPair->first, p2 = pointPair->second;   // with time info
        long t1 = (p1 == nullptr || p1->isHeadPtr()) ? 0 : p1->getTimestamp();
        long t2 = (p2 == nullptr || p2->isTailPtr()) ? 0 : p2->getTimestamp();
        int n = EditOperation::computeAvailableInsertion(t1, t2);
        int availNum = MIN3(expectIncrease - total, avail_expect, n);
        if(availNum <= 1) {
            long t = t1 == 0 ? t2 - 1 : (t2 == 0 ? t1 + 1 : (t1 + t2) / 2);
            auto newP = new Point(t, targetPid);
            newP->insertAfter(p1);
            // update or create
            if (leftSeg == nullptr)
                leftSeg = new lSegment(leftPoint, targetP, make_pair(p1, newP));
            else
                leftSeg->insertNewPair(make_pair(p1, newP));
            if (rightSeg == nullptr)
                rightSeg = new lSegment(targetP, rightPoint, make_pair(newP, p2));
            else
                rightSeg->insertNewPair(make_pair(newP, p2));
        }
        else {
            float gap = (float) (t2 - t1) / (float) (availNum + 1);
            Point *newP = nullptr;
            auto preP = p1;

            for(int i = 1; i <= availNum; i++) {
                long t = MIN(t1 + gap * i, t2 - 1);
                newP = new Point(t, targetPid);
                newP->insertAfter(preP);
                if(i == 1) { // the first segment
                    if (leftSeg == nullptr)
                        leftSeg = new lSegment(leftPoint, targetP, make_pair(p1, newP));
                    else
                        leftSeg->insertNewPair(make_pair(p1, newP));
                }
                else {
                    if (samePointSeg == nullptr)
                        samePointSeg = new lSegment(targetP, targetP, make_pair(preP, newP));
                    else
                        samePointSeg->insertNewPair(make_pair(preP, newP));
                }
                preP = newP;
            }
            // the last segment
            if (rightSeg == nullptr)
                rightSeg = new lSegment(targetP, rightPoint, make_pair(newP, p2));
            else
                rightSeg->insertNewPair(make_pair(newP, p2));
        }
        total += availNum;
        pointPair = trajPointPairs.erase(pointPair);
    }

    return make_tuple(leftSeg, rightSeg, samePointSeg);
}

void lSegment::setNull() {
    Segment::setNull();

    trajPointPairs.clear();
    trajPointPairs.shrink_to_fit();
}
