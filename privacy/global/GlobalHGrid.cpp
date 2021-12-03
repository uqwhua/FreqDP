//
// Created by Fengmei Jin on 14/11/21.
//

#include "GlobalHGrid.h"

GlobalHGrid::GlobalHGrid(Grid *_grid, int _granularity, bool forDeletion) : HierarGrid(_grid, _granularity) {
    const int totalCell = granularity * granularity;
    if(forDeletion) {
        gid2pid2uid2points.reserve(totalCell);
        for(int i = 0; i < totalCell; i++) {
            gid2pid2uid2points.emplace_back(unordered_map<int, map<string, vector<Point *>>>());
        }
    }
    else {
        map<int, gGridCell *> tmp;
        for (int row = 0; row < granularity; row++) {
            float lat_min = latitude_range[0] + step[0] * (float) row;
            float lat_max = lat_min + step[0];
            for (int col = 0; col < granularity; col++) {
                int zid = getZvalueByRowCol(row, col);
                auto gc = new gGridCell(zid, granularity);
                gc->setRange(lat_min, lat_max, longitude_range[0] + step[1] * (float) col,
                             longitude_range[0] + step[1] * (float) (col + 1));
                tmp[zid] = gc;
            }
        }
        for (int zid = 0; zid < totalCell; zid++) {
            allGrids.emplace_back(tmp[zid]);
        }
        tmp.clear();

    }
}

gGridCell *GlobalHGrid::getGridCellByZID(int zid) {
    return allGrids[zid];
}

void GlobalHGrid::connectWithLowerGrid(GlobalHGrid *lowerHG) {
    for(auto gc_parent: allGrids) {
        string str = gc_parent->getBaseFourStr();
        for(char c = '0'; c < '4'; c++) {
            int zid = GridCell::transferBaseFourToTen(str + c);
            auto gc = lowerHG->getGridCellByZID(zid);
            gc->setParent(gc_parent);
            gc_parent->addChild(gc);
        }
    }
}

void GlobalHGrid::insertSegment(const string &uid, int zid, Point *leftPoint, GeoPoint *lGP, Point *rightPoint, GeoPoint *rGP) {
    allGrids[zid]->insertSegment(new gSegment(lGP, rGP, uid, make_pair(leftPoint, rightPoint)));
}

void GlobalHGrid::computeZgridForPoints(const string &uid, const Trip *trip, StaticGrid *sgrid) {
    auto p = trip->getFirstPoint();
    while (p != nullptr && !p->isTailPtr()) {
        int pid = p->getPointId();
        auto rtn = locateGeoPoint(sgrid->getGeoPointById(pid));
        if(rtn != nullptr) {
            int zvalue = rtn[0];
            gid2pid2uid2points[zvalue][pid][uid].emplace_back(p);
        }
        p = p->getNextPoint();
    }
}

bool compUsers(pair<string, vector<Point*>> &u1, pair<string, vector<Point*>> &u2){
    return u1.second.size() < u2.second.size();
}

/** for Global mechanism, single point deletion */
void GlobalHGrid::selectTopUsersForGlobalPointDeletion(int targetGridId, int targetPid, int expectDecrease, StaticGrid *sgrid,
                                                       vector<pair<string, float>> &user2costs) {
    float kCost = INFINITY;
    bool sorted = false;
    auto targetGrid = gid2pid2uid2points[targetGridId];
    if(!targetGrid.empty()) {
        auto targetPoint = targetGrid.find(targetPid);
        if(targetPoint != targetGrid.end()) {
            vector<pair<string, vector<Point*>>> user2points;
            for(auto userPair: targetPoint->second) {
                user2points.emplace_back(userPair);
            }
            sort(user2points.begin(), user2points.end(), compUsers);    // sorted by the number of points for each user

            for(const auto& userPair: user2points) {
                string uid = userPair.first;
                float currCost = 0;
                for(auto point: userPair.second) {     // scan its all points
                    currCost += EditOperation::computeUtilityLossForPoint(sgrid, point);
                    if(currCost > kCost) {  // earlier terminate
                        currCost = -1;
                        break;
                    }
                }
                if(currCost >= 0) {
                    if(sorted && currCost < kCost) {
                        user2costs.pop_back();
                        if(user2costs.empty() || user2costs.back().second <= currCost) {
                            user2costs.emplace_back(make_pair(uid, currCost));
                        }
                        else {
                            int i = (int) user2costs.size() - 1;
                            while(i >= 0 && user2costs[i].second > currCost) {
                                i--;
                            }
                            user2costs.insert(user2costs.begin() + i + 1, make_pair(uid, currCost));
                        }
                        kCost = user2costs.back().second;
                    }
                    else
                        user2costs.emplace_back(make_pair(uid, currCost));
                }

                // if already found enough, then we have chance to earlier terminate
                if(!sorted && user2costs.size() >= expectDecrease) {
                    sorted = true;
                    sort(user2costs.begin(), user2costs.end(), EditOperation::compareCost<string>); // once
                    while (user2costs.size() > expectDecrease) {
                        user2costs.pop_back();
                    }
                    kCost = user2costs.back().second;   // first update
                }
            }

            user2points.clear();
        }
    }
}

map<string, int> GlobalHGrid::completeDeletePointFromGrid(int targetPid, int targetGridId, set<string> &selectedUsers) {
    map<string, int> user2deleteNum;
    auto targetGrid = gid2pid2uid2points[targetGridId];
    if(!targetGrid.empty()) {
        auto targetPoint = targetGrid.find(targetPid);
        if(targetPoint != targetGrid.end()) {
            auto user_itr = targetPoint->second.begin();
            while (user_itr != targetPoint->second.end()) {
                string uid = user_itr->first;
                auto itr = selectedUsers.find(uid);
                if(itr != selectedUsers.end() || selectedUsers.empty()) {   // if empty, then all users
                    int deleteNum = 0;
                    // delete all
                    for(auto p: user_itr->second) {
                        p->unlink();
                        p->clear();
                        delete p;
                        p = nullptr;
                        deleteNum++;
                    }
                    user2deleteNum[uid] = deleteNum;
                    user_itr = targetPoint->second.erase(user_itr);
                    if(!selectedUsers.empty())
                        selectedUsers.erase(itr);
                }
                else {
                    user_itr++;
                }
            }
        }
    }

    return user2deleteNum;
}

void GlobalHGrid::clearUselessInfo() {
    gid2pid2uid2points.clear();
}

void GlobalHGrid::clearGrids() {
    HierarGrid::clearGrids();

    for(auto gc: allGrids) {
        gc->clearAll();
        gc = nullptr;
    }
    allGrids.clear();
}
