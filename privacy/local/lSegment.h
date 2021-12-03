//
// Created by Fengmei Jin on 26/11/21.
//

#ifndef HIERARCHICALGRID_LSEGMENT_H
#define HIERARCHICALGRID_LSEGMENT_H

#include "../Segment.h"

class lSegment: public Segment{

    vector<pair<Point*, Point*>> trajPointPairs;    // all segment with same spatial but different temporal info

public:
    // constructor for LOCAL
    lSegment(GeoPoint *&leftP, GeoPoint *&rightP, pair<Point*, Point*> timePair): Segment(leftP, rightP){
        trajPointPairs.emplace_back(timePair);
    }

    void setNull() override;

    bool empty() override;

    void merge(Segment *seg) override;

    void insertNewPair(pair<Point *, Point *> newPair);

    [[nodiscard]] unsigned int getPointPairNum() const override;

    [[nodiscard]] int computeAvailableInsertion() const;    // depends on avg sampling rate

    tuple<lSegment *, lSegment *, lSegment *> split_insert(int targetPid, GeoPoint *targetP, int avail_expect, int expectIncrease);


    // ****************** Used in Local Insertion
    struct lSegNode{
        lSegment* _seg;
        float _dist;
        int _granularity;
        int _zid;

        int _availNum{0};      // how many available selected for this segment

        lSegNode(lSegment *seg, float dist, int level, int zid, int num): _seg(seg), _dist(dist), _granularity(level), _zid(zid), _availNum(num) {}

        lSegNode(lSegment *seg, float dist, int level, int zid): _seg(seg), _dist(dist), _granularity(level), _zid(zid) {}
    };

    struct lSegNodeComp {
        bool operator() (lSegNode &s1, lSegNode &s2) {
            return s1._dist < s2._dist;     // --> the larger distance will be the top of queue
        }
    };

    typedef priority_queue<lSegNode, vector<lSegNode>, lSegNodeComp> SegQueue;
};


#endif //HIERARCHICALGRID_LSEGMENT_H
