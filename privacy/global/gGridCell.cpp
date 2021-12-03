//
// Created by Fengmei Jin on 14/11/21.
//

#include "gGridCell.h"

bool gGridCell::isValid() const {
    return !segments.empty();
}

void gGridCell::clearSegments() {
    for(const auto& user: segments) {
        for (auto seg: user.second) {
            seg->setNull();
            delete seg;
            seg = nullptr;
        }
    }
    segments.clear();
}

void gGridCell::clearAll() {
    GridCell::clearAll();
    clearSegments();
}

void gGridCell::insertSegment(gSegment *seg) {
    int idx = seg->getStartPointId();
    auto itr = segments.find(idx);
    if(itr != segments.end()) {
        for(auto s: itr->second) {
            if(s->equal(seg)) {     // spatially equal
                s->merge(seg);
                delete seg;
                seg = nullptr;
                return;
            }
        }
    }
    else {
        segments[idx].emplace_back(seg);
    }
}

// the segment queue here should: 1) distinct users; 2) have distance priority to support pruning
long gGridCell::checkSegmentsForInsertOnce(int targetPid, const GeoPoint *targetP, StaticGrid *sgrid, const unordered_set<string> &unavailableUsers,
                                           gSegment::SegUserQueue &queue, unordered_map<string, gSegment::SegUserNode *> &selectedUsers, int expectIncrease) {
    int segCheck = 0;
    for(const auto& startPoint: segments) {
        const float distToLeft = sgrid->getPairwiseDist(targetPid, startPoint.first);       // the distance from query point to the left-point of this segment is fixed

        for(auto seg: startPoint.second) {
            segCheck++;

            float dist = seg->computeDistance(targetPid, targetP, sgrid, distToLeft);

            // only in this case, the queue will be updated
            if(queue.size() < expectIncrease || dist < queue.top()->minimumDist()) {

                seg->checkTrajPointPairs(dist, this->granularity, this->zvalue, unavailableUsers, queue, selectedUsers, expectIncrease);
            }
        }
    }
    return segCheck;
}

void gGridCell::removeSegment(gSegment *seg) {
    int idx = seg->getStartPointId();
    auto itr = segments.find(idx);
    if(itr != segments.end()) {
        auto sitr = itr->second.begin();
        while(sitr != itr->second.end()) {
            if((*sitr) == seg) {    // this seg must be already empty
                itr->second.erase(sitr);
                seg->setNull();
                delete seg;
                seg = nullptr;
                break;
            }
            else
                sitr++;
        }
        if(itr->second.empty()){
            segments.erase(itr);
        }
    }
}

unsigned int gGridCell::getSegmentNum(bool trajSeg) const {
    unsigned int total = 0;
    for(const auto& sp: segments){
        if(!trajSeg) {
            total += sp.second.size();
        }
        else {
            for (auto seg: sp.second) {
                total += seg->getPointPairNum();
            }
        }
    }
    return total;
}
