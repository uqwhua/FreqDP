//
// Created by Fengmei Jin on 26/11/21.
//

#include "gSegment.h"

bool gSegment::empty() {
    return user2trajPointPair.empty();
}

void gSegment::merge(Segment *seg) {
    auto *gSeg = dynamic_cast<gSegment *>(seg);
    for(const auto& eachUser: gSeg->user2trajPointPair) {
        string uid = eachUser.first;
        auto itr = this->user2trajPointPair.find(uid);
        if(itr == this->user2trajPointPair.end()) {
            this->user2trajPointPair.insert(eachUser);      // this is a new user-segment
        }
        else {
            itr->second.insert(itr->second.end(), eachUser.second.begin(), eachUser.second.end());
        }
    }
}

void gSegment::setNull() {
    Segment::setNull();

    for(auto pair: user2trajPointPair) {
        pair.second.clear();
        pair.second.shrink_to_fit();
    }
    user2trajPointPair.clear();
}

pair<gSegment *, gSegment *> gSegment::split_insert(int targetPid, GeoPoint *targetP, const string &uid) {
    gSegment *leftSeg = nullptr, *rightSeg = nullptr;

    auto uitr = user2trajPointPair.find(uid);
    if(uitr != user2trajPointPair.end()) {
        auto pointPairs = uitr->second;
        if(!pointPairs.empty()) {
            auto itr = pointPairs.back();
            auto p1 = itr.first, p2 = itr.second;   // with time info
            long t1 = (p1 == nullptr || p1->isHeadPtr()) ? 0 : p1->getTimestamp();
            long t2 = (p2 == nullptr || p2->isTailPtr()) ? 0 : p2->getTimestamp();
            long t = t1 == 0 ? t2 - 1 : (t2 == 0 ? t1 + 1 : (t1 + t2) / 2);
            auto newP = new Point(t, targetPid);
            newP->insertAfter(p1);

            leftSeg = new gSegment(leftPoint, targetP, uid, make_pair(p1, newP));
            rightSeg = new gSegment(targetP, rightPoint, uid, make_pair(newP, p2));

            uitr->second.pop_back();    // this pair has been split

            if(uitr->second.empty()) {
                user2trajPointPair.erase(uitr);
            }
        }
    }

    return make_pair(leftSeg, rightSeg);
}

void gSegment::checkTrajPointPairs(float dist, int granularity, int zid, const unordered_set<string> &unavailableUsers,
                                   gSegment::SegUserQueue &queue, unordered_map<string, gSegment::SegUserNode *> &selectedUsers, int expectIncrease) {

    auto *currNode = new gSegment::gSegNode(this, dist, granularity, zid);    // all these users will share this node

    auto userForCurrSeg = this->user2trajPointPair.begin();
    while (userForCurrSeg != this->user2trajPointPair.end()){
        if(unavailableUsers.find(userForCurrSeg->first) == unavailableUsers.end()) {  // this is an available user
            string uid = userForCurrSeg->first;

            auto uitr = selectedUsers.find(uid);
            if(uitr != selectedUsers.end() && uitr->second != nullptr) {   // already exist in the queue
                if(dist < uitr->second->minimumDist()) {
                    uitr->second->pushSegNode(currNode);
                }
            }
            else {  // this user didn't exist in the queue before
                if(queue.size() >= expectIncrease && dist < queue.top()->minimumDist()){
                    string id = queue.top()->_uid;
                    queue.pop();
                    delete selectedUsers[id];
                    selectedUsers[id] = nullptr;
                    selectedUsers.erase(id);
                }
                auto segnode = new gSegment::SegUserNode(currNode, uid);
                queue.push(segnode);
                selectedUsers[uid] = segnode;   // insert a new one
            }
        }
        userForCurrSeg++;
    }
}

unsigned int gSegment::getPointPairNum() const {
    unsigned int total = 0;
    for(const auto& pair: user2trajPointPair) {
        total += pair.second.size();
    }
    return total;
}
