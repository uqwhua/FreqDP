//
// Created by Fengmei Jin on 14/11/21.
//

#include "lGridCell.h"

bool lGridCell::isValid() const {
    return !segments.empty();
}

unsigned int lGridCell::getSegmentNum(bool trajSeg) const {
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


// the special processing after queueTotal >= expectInsert
int updateSegQueue(lSegment::SegQueue &queue, lSegment *seg, float dist, int granularity, int zid, int availNum_expect,
                   int queueTotal, int expectInsert){
    auto topK_cur = queue.top();
    float kDist = topK_cur._dist;
    bool segAdd = true;

    if(dist < kDist) {      // prefer to reserve the closer one
        queue.pop();
        queueTotal -= topK_cur._availNum;
    }
    else if(dist == kDist) {        // with same distance, become topK_cur together if possible
        if(queue.size() >= expectInsert) {
            if(topK_cur._availNum > availNum_expect) {
                segAdd = false;
            }
            else if (topK_cur._availNum == availNum_expect && topK_cur._granularity < granularity) {   // long-length segment
                segAdd = false;
            }
            else {
                queue.pop();
                queueTotal -= topK_cur._availNum;
            }
        }
        else {        // else reserve both with same distance
            queue.pop();
            queueTotal -= topK_cur._availNum;
            topK_cur._availNum /= 2;    // shrink the availNum for the current top-k
            if(topK_cur._availNum > 0) {
                queue.push(topK_cur);
                queueTotal += topK_cur._availNum;
            }
        }
    }
    if(segAdd) {
        int availNum_shrink = MIN(availNum_expect, expectInsert - queueTotal);
        queue.push(lSegment::lSegNode(seg, dist, granularity, zid, availNum_shrink));
        queueTotal += availNum_shrink;
    }

    return queueTotal;
}

int lGridCell::checkSegmentsForPointInsertion(int targetPid, const GeoPoint *targetP, StaticGrid *sgrid,
                                               lSegment::SegQueue &queue, int* num) {
    int queueTotal = num[0], expectInsert = num[1];
    int segCheck = 0;
    for(const auto& startPoint: segments) {
        const float distToLeft = sgrid->getPairwiseDist(targetPid, startPoint.first);       // the distance from query point to the left-point of this segment is fixed
        for (auto seg: startPoint.second) {
            segCheck++;

            float dist = seg->computeDistance(targetPid, targetP, sgrid, distToLeft);   // shortest distance from a targetP to a segment
            int availNum_expect = seg->computeAvailableInsertion();

            if(queueTotal < expectInsert) {
                int availNum_shrink = MIN(availNum_expect, expectInsert - queueTotal);        // bounded by the global available number
                queue.push(lSegment::lSegNode(seg, dist, this->granularity, this->zvalue, availNum_shrink));
                queueTotal += availNum_shrink;
            }
            else if(dist <= queue.top()._dist){
                queueTotal = updateSegQueue(queue, seg, dist, this->granularity, this->zvalue, availNum_expect, queueTotal, expectInsert);
            }
        }
    }
    num[0] = queueTotal;    // update
    return segCheck;
}

void lGridCell::insertSegment(lSegment *seg) {
    int idx = seg->getStartPointId();
    auto itr = segments.find(idx);
    if(itr != segments.end()) {
        for(auto s: itr->second) {
            if(s->equal(seg)) {
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

void lGridCell::removeSegment(lSegment *seg) {
    int idx = seg->getStartPointId();
    auto itr = segments.find(idx);
    if(itr != segments.end()) {
        auto sitr = itr->second.begin();
        while(sitr != itr->second.end()) {
            if((*sitr) == seg) {
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

bool lGridCell::removeSegments(set<lSegment *> &toBeRemoved) {
    auto itr = segments.begin();
    while (itr != segments.end() && !toBeRemoved.empty()) {
        auto seg = itr->second.begin();
        while(seg != itr->second.end() && !toBeRemoved.empty()) {
            auto found = toBeRemoved.find(*seg);
            if(found != toBeRemoved.end()) {
                delete *seg;
                *seg = nullptr;
                seg = itr->second.erase(seg);
                toBeRemoved.erase(found);
            }
            else
                seg++;
        }
        if(itr->second.empty()){
            itr = segments.erase(itr);
        }
        else
            itr++;
    }
    return segments.empty();
}

void lGridCell::clearSegments() {
    for(auto& startP: segments) {
        for (auto &seg: startP.second) {
            seg->setNull();
            delete seg;
            seg = nullptr;
        }
        startP.second.clear();
        startP.second.shrink_to_fit();
    }
    segments.clear();
}

void lGridCell::clearAll() {
    GridCell::clearAll();
    if(!segments.empty())
        clearSegments();
}

//float lGridCell::findNearestSegment(int targetPid, const GeoPoint *targetP, const string &uid, StaticGrid *sgrid,
//                                     lSegment::lSegNode *&curNearestSeg, float curNearestDist) {
//    for(const auto& startPoint: segments) {
//        const float distToLeft = sgrid->getPairwiseDist(targetPid, startPoint.first);       // the distance from query point to the left-point of this segment is fixed
//        for (auto seg: startPoint.second) {
//
//            float dist = seg->computeDistance(targetPid, targetP, sgrid, distToLeft);   // shortest distance from a targetP to a segment
//
//            if(dist < curNearestDist) {
//                curNearestSeg = new lSegment::lSegNode(seg, dist, this->granularity, this->zvalue);
//                curNearestDist = dist;
//            }
//        }
//    }
//    return curNearestDist;
//}