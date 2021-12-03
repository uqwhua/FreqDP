//
// Created by Fengmei Jin on 14/11/21.
//

#include "LocalHGrid.h"
#include "../../Utils.h"

LocalHGrid::LocalHGrid(Grid *grid, int granularity, bool forDeletion) : HierarGrid(grid, granularity) {
    const int totalCell = granularity * granularity;
    if(forDeletion) {
        gid2pid2points.reserve(totalCell);
        for(int i = 0; i < granularity * granularity; i++) {
            gid2pid2points.emplace_back(unordered_map<int, vector<Point *>>());
        }
    }
    else {
        map<int, lGridCell *> tmp;
        for (int row = 0; row < granularity; row++) {
            float lat_min = latitude_range[0] + step[0] * (float) row;
            float lat_max = lat_min + step[0];
            for (int col = 0; col < granularity; col++) {
                int zid = getZvalueByRowCol(row, col);
                auto gc = new lGridCell(zid, granularity);
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

void LocalHGrid::clearUselessInfo() {
    gid2pid2points.clear();
}

void LocalHGrid::clearContent() {
    for(auto gc: allGrids) {
        gc->clearSegments();
        gc->clearValidPointers();
    }
}

void LocalHGrid::clearGrids() {
    HierarGrid::clearGrids();

    for(auto gc: allGrids) {
        gc->clearAll();
        gc = nullptr;
    }
    allGrids.clear();
}

lGridCell *LocalHGrid::getGridCellByZID(int zid) {
    return allGrids[zid];
}

void LocalHGrid::insertSegment(int zid, Point *leftPoint, GeoPoint *lGP, Point *rightPoint, GeoPoint *rGP){
    allGrids[zid]->insertSegment(new lSegment(lGP, rGP, make_pair(leftPoint, rightPoint)));
}

// split a segment into two parts by inserting the target point in the middle
// find where the new segment should be
// @zid: the grid cell where these segment locates
unsigned int LocalHGrid::splitSegments(int targetPid, GeoPoint *targetP, int expectIncrease, int zid, const vector<pair<lSegment *, int>>& candidateSegs,
                                       map<int, map<int, vector<lSegment *>>> &level2zid2segments, long *updateTimeTotal, bool compressed) {
    vector<lSegment*> newSegs_cur;
    const int *targetZ_finest = locateGeoPoint(targetP, FINEST_GRANULARITY);
    int zFinest = targetZ_finest[0];
    unsigned int success = 0;

    long updateTimer, total = 0;
    for (auto seg: candidateSegs) {

        auto newSegs = seg.first->split_insert(targetPid, targetP, seg.second, expectIncrease);     // insertion to the trajectory

        updateTimer = Utils::millisecond();      // below are for the grid update

        auto segLeft = get<0>(newSegs);
        int *segGZ = findBestFitGrid(segLeft, nullptr, targetZ_finest);
        if (segGZ != nullptr) {
            int l = segGZ[0], z = segGZ[1];
            if (l == this->granularity && z == zid) {
                newSegs_cur.emplace_back(segLeft);
            }
            else {
                level2zid2segments[l][z].emplace_back(segLeft);
            }
        }

        auto segRight = get<1>(newSegs);
        segGZ = findBestFitGrid(segRight, targetZ_finest, nullptr);
        if (segGZ != nullptr) {
            int l = segGZ[0], z = segGZ[1];
            if (l == this->granularity && z == zid) {
                newSegs_cur.emplace_back(segRight);
            }
            else {
                level2zid2segments[l][z].emplace_back(segRight);
            }
        }
        success += segLeft->getPointPairNum();

        auto samePointSeg = get<2>(newSegs);
        if (samePointSeg != nullptr) {
            level2zid2segments[FINEST_GRANULARITY][zFinest].emplace_back(samePointSeg);
            success += samePointSeg->getPointPairNum();
        }

        if(seg.first->empty())
            allGrids[zid]->removeSegment(seg.first);

        total += (Utils::millisecond() - updateTimer);
    }

    updateTimer = Utils::millisecond();

    auto gc = allGrids[zid];
    for (auto seg: newSegs_cur)
        gc->insertSegment(seg);

    if(compressed && !gc->isValid()) {
        gc->unlinkSelf();
    }

    total += (Utils::millisecond() - updateTimer);
    *updateTimeTotal += total;

    return success;
}

/************************************************ used in Local mechanism **********************************************
 * Deletion only performed on finest grid, point-based
 * Insertion needs the hierarchical structure
 * */

void LocalHGrid::computeZgridForPoints(const Trip *trip, StaticGrid *sgrid) {
    auto p = trip->getFirstPoint();
    while (p != nullptr && !p->isTailPtr()) {
        int pid = p->getPointId();
        auto rtn = locateGeoPoint(sgrid->getGeoPointById(pid));
        if(rtn != nullptr) {
            int zvalue = rtn[0];
            gid2pid2points[zvalue][pid].emplace_back(p);
        }
        p = p->getNextPoint();
    }
}

int LocalHGrid::completeDeletePoint(int targetPid, int targetGid) {
    int deleteNum = 0;
    auto targetGrid = gid2pid2points.at(targetGid);
    if (!targetGrid.empty()) {
        auto targetPointContent = targetGrid.find(targetPid);
        if (targetPointContent != targetGrid.end()) {
            auto pitr = targetPointContent->second.begin();    // wit the same pid
            while (pitr != targetPointContent->second.end()) {
                (*pitr)->unlink();
                (*pitr)->clear();
                delete *pitr;
                *pitr = nullptr;
                pitr++;
                deleteNum++;
            }
            targetGrid.erase(targetPointContent);    // this point is completely deleted
        }
    }
    return deleteNum;
}

bool compCost(tuple<Point *, int, float> &t1, tuple<Point *, int, float> &t2) {
    return get<2>(t1) < get<2>(t2);     // ascending cost
}

bool compPos(tuple<Point *, int, float> &t1, tuple<Point *, int, float> &t2) {
    return get<1>(t1) > get<1>(t2);     // descending position
}

int LocalHGrid::selectiveDeletePoint(int targetPid, int targetGid, int expectedDelete, StaticGrid *sgrid) {
    int remainingNum = abs(expectedDelete);
    auto targetGrid = gid2pid2points.at(targetGid);
    if (!targetGrid.empty()) {
        auto itr2 = targetGrid.find(targetPid);
        if (itr2 != targetGrid.end()) {
            vector<tuple<Point*, int, float>> sortedCosts;
            vector<tuple<Point*, int, float>> zeroCosts;

            for (int i = 0; i < itr2->second.size(); i++) {
                auto p = itr2->second[i];
                float cost = EditOperation::computeUtilityLossForPoint(sgrid, p);
                if(cost == 0)
                    zeroCosts.emplace_back(make_tuple(p, i, 0));
                else
                    sortedCosts.emplace_back(make_tuple(p, i, cost));
            }

            int lackNum = remainingNum - (int) zeroCosts.size();
            if(lackNum > 0) {
                // sort by ascending cost
                if(sortedCosts.size() > lackNum) {
                    sort(sortedCosts.begin(), sortedCosts.end(), compCost);
                    int i = 0;
                    while (lackNum-- > 0 && i < sortedCosts.size()) {
                        zeroCosts.emplace_back(sortedCosts[i++]);
                    }
                }
                else {
                    zeroCosts.insert(zeroCosts.end(), sortedCosts.begin(), sortedCosts.end());  // all needed
                }
            } // hereafter, zeroCosts maybe not zero costs

            // sort by descending position
            sort(zeroCosts.begin(), zeroCosts.end(), compPos);

            auto vecBegin = itr2->second.begin();
            for (auto tp: zeroCosts) {
                Point *toDelete = get<0>(tp);
                toDelete->unlink();
                toDelete->clear();
                delete toDelete;
                toDelete = nullptr;

                int pos = get<1>(tp);
                itr2->second.erase(vecBegin + pos);
                if(--remainingNum <= 0) {
                    break;
                }
            }
            sortedCosts.clear();
            sortedCosts.shrink_to_fit();
            zeroCosts.clear();
            zeroCosts.shrink_to_fit();

            if (itr2->second.empty())    // should not happen
                targetGrid.erase(itr2);
        }
    }
    return abs(expectedDelete) - remainingNum;
}

// structurally connect, a full grid
void LocalHGrid::connectWithLowerGrid(LocalHGrid *lowerHG) {
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
