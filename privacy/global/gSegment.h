//
// Created by Fengmei Jin on 26/11/21.
//

#ifndef HIERARCHICALGRID_GSEGMENT_H
#define HIERARCHICALGRID_GSEGMENT_H

#include "../Segment.h"

class gSegment : public Segment{

    unordered_map<string, vector<pair<Point*, Point*>>> user2trajPointPair;

public:
    gSegment(GeoPoint *&leftP, GeoPoint *&rightP, const string& uid, pair<Point*, Point*> timePair): Segment(leftP, rightP) {
        user2trajPointPair[uid].emplace_back(timePair);
    }

    bool empty() override;

    void setNull() override;

    void merge(Segment *seg) override;

    [[nodiscard]] unsigned int getPointPairNum() const override;

    pair<gSegment *, gSegment *> split_insert(int targetPid, GeoPoint *targetP, const string &uid);

    // ****************** Used in GLOBAL Mechanism
    struct gSegNode{
        gSegment *_seg;
        float _dist;
        int _granularity;
        int _zid;
        gSegNode(gSegment *seg, float dist, int level, int zid): _seg(seg), _dist(dist), _granularity(level), _zid(zid) {}

    };

    struct gSegNodeCompReverse {
        bool operator() (gSegNode *s1, gSegNode *s2) {
            return s1->_dist > s2->_dist;
        }
    };

    struct SegUserNode {
        priority_queue<gSegNode *, vector<gSegNode*>, gSegNodeCompReverse> _queue;    // all possible segments for this user sorted by the distance
        string _uid;

        SegUserNode(gSegNode *node, const string &uid) {
            _uid = uid;
            _queue.push(node);
        }
        [[nodiscard]] float minimumDist() const {
            return _queue.top()->_dist;
        }
        [[nodiscard]] gSegNode* topOneSegNode() const {
            return _queue.top();
        }
        void pushSegNode(gSegNode *node){
            _queue.push(node);
        }
        void clearAll() {
            while (!_queue.empty()) {
                auto toDelete = _queue.top();
                toDelete = nullptr;
                _queue.pop();
            }
        }
    };

    struct SegUserComp {
        bool operator() (SegUserNode *s1, SegUserNode *s2) {
            return s1->minimumDist() < s2->minimumDist();
        }
    };

    typedef priority_queue<SegUserNode*, vector<SegUserNode*>, SegUserComp> SegUserQueue;

    void checkTrajPointPairs(float dist, int granularity, int zid, const unordered_set<string> &unavailableUsers,
                             SegUserQueue &queue, unordered_map<string, gSegment::SegUserNode *> &selectedUsers,
                             int expectIncrease);
};


#endif //HIERARCHICALGRID_GSEGMENT_H
