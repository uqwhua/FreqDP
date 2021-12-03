//
// Created by s4570405 on 28/06/2021.
//

#include "LocalEdit.h"
#include "../../Utils.h"

void LocalEdit::modify(const map<string, FrequencyVec *> &allFreqVec_ori, const map<string, FrequencyVec *> &allFreqVec_pert,
                       StaticGrid *sgrid, map<string, Trip *> &trajectories, long *timeCosts) {
    unsigned int total = allFreqVec_ori.size();
    int cnt = 0;

    printf("[PROGRESS] ");
    fflush(stdout);
    long totalDecreaseTime = 0, totalIncreaseTime = 0, sortTime = 0, insertTime = 0, updateGridTime = 0;

    long startTimer = Utils::millisecond();
    vector<LocalHGrid*> multiGrids;
    buildGridStructure(sgrid, multiGrids);
    long gridConstructionTime = Utils::millisecond() - startTimer;

    map<int, int> point2ori2diff;
    for (const auto &fv_pair: allFreqVec_ori) {
        string userID = fv_pair.first;
        FrequencyVec *freqVec_ori = fv_pair.second;
        auto itr = allFreqVec_pert.find(userID);
        if (itr != allFreqVec_pert.end()) {
            FrequencyVec *freqVec_pert = itr->second;

            auto trajectory = trajectories[userID];

            // construct the finest grid for the current trajectory before deletion
            startTimer = Utils::millisecond();
            auto finestGrid = new LocalHGrid(sgrid, FINEST_GRANULARITY, true);
            finestGrid->computeZgridForPoints(trajectory, sgrid);
            gridConstructionTime += (Utils::millisecond() - startTimer);

            // scan all points
            int totalPoints = freqVec_pert->getArraryLen();
            for (int pid = 0; pid < totalPoints; pid++) {
                int freq_ori = freqVec_ori->getValueByKey(pid);

                /* A heuristic strategy: first deletion, then insertion */
                int diff = freqVec_pert->getValueByKey(pid) - freq_ori;
                if (diff != 0) {
                    if (diff < 0) {
                        startTimer = Utils::millisecond();
                        reduceOccurrence(pid, freq_ori, diff, userID, sgrid, finestGrid, trajectory);
                        totalDecreaseTime += (Utils::millisecond() - startTimer);
                    }
                    else {
                        // insertion will be done after deletion
                        point2ori2diff[pid] = diff;
                    }
                }
            }

            // construct hierarchical grids based on the finest granularity
            finestGrid->clearUselessInfo();
            finestGrid->clearGrids();       // become useless during insertion
            delete finestGrid;
            finestGrid = nullptr;

            startTimer = Utils::millisecond();
            vector<unordered_set<int>> validGridCells = buildGridContent(sgrid, multiGrids, trajectory);
            compressGridCells(multiGrids, validGridCells);      // compress those empty grid cell with no valid segments
            gridConstructionTime += (Utils::millisecond() - startTimer);


            validGridCells.clear();
            validGridCells.shrink_to_fit();

            // insertion for each point
            for (auto eachPoint: point2ori2diff) {
                startTimer = Utils::millisecond();
                int pid = eachPoint.first, increaseNum = eachPoint.second;
                auto rtn = increaseOccurrence(pid, increaseNum, userID, sgrid, multiGrids, trajectory);
                totalIncreaseTime += (Utils::millisecond() - startTimer);
                sortTime += rtn[1];
                insertTime += rtn[2];
                updateGridTime += rtn[3];

                delete [] rtn;
                rtn = nullptr;
            }

            point2ori2diff.clear();
            Utils::clearGrids(multiGrids, false);
        }

        // print out progress
        if (total >= 10 && ++cnt % (total / 10) == 0) {
            printf("%d%%...", (int) (cnt * 100.0 / total));
            fflush(stdout);
        }
    }

    printf("done!\n");

    timeCosts[2] = totalDecreaseTime;
    timeCosts[3] = totalIncreaseTime;
    timeCosts[4] = sortTime;
    timeCosts[5] = insertTime;
    timeCosts[6] = updateGridTime;
    timeCosts[7] = gridConstructionTime;    // total

    Utils::clearGrids(multiGrids);  // completely delete the grids
}

int LocalEdit::reduceOccurrence(int targetPointId, int oriFreqValue, int expectedDelete, const string &userID,
                                StaticGrid *sgrid, LocalHGrid *finestGrid, Trip *trajectory) {

    int targetFreqValue = oriFreqValue + expectedDelete; // expectedDelete < 0
    GeoPoint *geopoint = sgrid->getGeoPointById(targetPointId);

    int deletedNum = 0;
    auto rtn = finestGrid->locateGeoPoint(geopoint);
    if (rtn != nullptr) {
        int gZ = rtn[0];

        if (targetFreqValue == 0) {
            // directly delete all from the user's trip, no need to compute any utility loss, same as the global edit
            deletedNum = finestGrid->completeDeletePoint(targetPointId, gZ);
        } else {
            deletedNum = finestGrid->selectiveDeletePoint(targetPointId, gZ, expectedDelete, sgrid);
        }

        if (deletedNum > 0) {
            trajectory->decreaseLength(deletedNum);
        }
    }

    return expectedDelete + deletedNum;
}

int searchSegmentsBottomUpDown(int targetPointID, const GeoPoint *targetP, int expectIncrease, StaticGrid *sgrid, const vector<LocalHGrid *> &multiGrids,
                               lSegment::SegQueue &candidateSegs, long *&statistics, bool compressed = true){

    vector<bool*> visited;
    for(auto hg: multiGrids) {
        int g = hg->getGranularity();
        visited.emplace_back(new bool[g*g]{});
    }

    stack<HierarGrid::QNode> candidateGrids_bottomup;   // used before root
    HierarGrid::GridQueue candidateGrids_topdown;       // used after root is accessed

    long startTimer = Utils::millisecond();

    // initialize the stack of candidate grid cells
    int idx = IDX(FINEST_GRANULARITY);
    int zid = multiGrids[idx]->locateGeoPoint(targetP)[0];
    auto startCell = multiGrids[idx]->getGridCellByZID(zid);    // to find the finest grid cell for the target point
    while (!startCell->isValid()) {
        startCell = dynamic_cast<lGridCell *>(startCell->getParent());
    }
    candidateGrids_bottomup.push(HierarGrid::QNode(startCell, 0));

    int *num = new int[2] {0, expectIncrease};   // num[0]: how many in the queue now
    float kDist = INFINITY;    // top-k segment distance, initially infinite
    bool rootChecked = false;

    long checkedCellNum = 0, prunedCellNum = 0, checkedSegNum = 0;

    // start search by first bottom-up then top-down
    lGridCell *candiGC;
    float mindist;
    while (!candidateGrids_bottomup.empty() || !candidateGrids_topdown.empty()) {
        if(!rootChecked) {
            auto candi = candidateGrids_bottomup.top();
            candidateGrids_bottomup.pop();

            candiGC = dynamic_cast<lGridCell *>(candi._gc);
            mindist = candi._minDist;

            // if already found k-segments
            if (num[0] >= expectIncrease){
                mindist = mindist >= 0 ? mindist : candiGC->computeMinDist(targetP);
                if (mindist > kDist){     // the current candidate cannot contain any promising segments
                    prunedCellNum++;
                    visited[IDX(candiGC->getGranularity())][candiGC->getID()] = true;
                    continue;
                }
            }
        }
        else {
            auto candi = candidateGrids_topdown.top();
            candidateGrids_topdown.pop();
            candiGC = dynamic_cast<lGridCell *>(candi._gc);

            // early terminate after root
            if (candi._minDist > kDist && num[0] >= expectIncrease) {     // hereafter, cannot exist promising segments
                break;
            }
        }

        checkedCellNum++;
        visited[IDX(candiGC->getGranularity())][candiGC->getID()] = true;
        if (candiGC->getGranularity() == COARSEST_GRANULARITY){
            rootChecked = true;
        }


        if(candiGC->isValid()) {    // own some segments inside
            checkedSegNum += candiGC->checkSegmentsForPointInsertion(targetPointID, targetP, sgrid, candidateSegs, num);
            if(num[0] >= expectIncrease) {
                kDist = candidateSegs.top()._dist;      // only after k-segments are found, this threshold will work
            }
        }

        // first push structuralParent during bottom-up
        if(!rootChecked) {
            auto gParent = candiGC->getParent(compressed);
            int g = gParent->getGranularity();
            if(!visited[IDX(g)][gParent->getID()]) {
                if (g == COARSEST_GRANULARITY) {    // after that, it would be top-down search
                    rootChecked = true;
                    candidateGrids_topdown.push(HierarGrid::QNode(gParent, 0));     // only once
                }
                else {
                    candidateGrids_bottomup.push(HierarGrid::QNode(gParent, 0));
                }
            }
        }

        // then push structuralChildren
        auto children = candiGC->getChildren(compressed);
        for (auto gChild: children) {
            if(!visited[IDX(gChild->getGranularity())][gChild->getID()]) {
                if(!rootChecked) {
                    candidateGrids_bottomup.push(HierarGrid::QNode(gChild, -1));     // calculate later if needed
                }
                else {
                    float dist = gChild->computeMinDist(targetP);
                    if(dist <= kDist)
                        candidateGrids_topdown.push(HierarGrid::QNode(gChild, dist));
                }
            }
        }
        candiGC = nullptr;
    }
    statistics[1] = Utils::millisecond() - startTimer;      // sort time

    for(auto pair: visited) {
        delete[] pair;
        pair = nullptr;
    }
    visited.clear();

    const int rtn = num[0];
    delete[] num;
    num = nullptr;

    targetP = nullptr;

    return rtn;  // the true size of seg-queue
}

unsigned int insertIntoSegments(int targetPointID, GeoPoint *targetP, int expectIncrease, vector<LocalHGrid *> &multiGrids,
                                lSegment::SegQueue &candidateSegs, map<int, map<int, vector<lSegment *>>> &granularity2zid2newSegments,
                                long *&statistics, bool compressed = true) {
    // a count step first
    map<int, map<int, vector<pair<lSegment *, int>>>> gIDX2zid2toSplitSegs;
    long startTimer = Utils::millisecond(), extraUpdateTime = 0;
    unsigned int success = 0;

    while (!candidateSegs.empty()) {
        auto candiS = candidateSegs.top();
        candidateSegs.pop();

        int granularity_idx = IDX(candiS._granularity);
        int zid = candiS._zid;              // this segment should entirely stay in this grid cell, at this granularity
        gIDX2zid2toSplitSegs[granularity_idx][zid].emplace_back(make_pair(candiS._seg, candiS._availNum));
    }

    // insert new point and generate more segments if needed
    for (const auto &eachGranularity: gIDX2zid2toSplitSegs) {
        auto hg = multiGrids[eachGranularity.first];
        for (const auto &eachCell: eachGranularity.second) {
            success += hg->splitSegments(targetPointID, targetP, expectIncrease, eachCell.first, eachCell.second, // all segments located in this grid eachCell at this granularity
                                         granularity2zid2newSegments, &extraUpdateTime, compressed);
            if (success >= expectIncrease) {
                break;
            }
        }
    }

    statistics[2] = Utils::millisecond() - startTimer - extraUpdateTime;      // insertion time
    statistics[3] = extraUpdateTime;    // time used for update grids

    gIDX2zid2toSplitSegs.clear();

    return success;
}

/** @param oriFreq: the original frequency of this point in the trajectory; used for debugging/checking result
 * */
long *LocalEdit::increaseOccurrence(int targetPointId, int expectedIncrease, const string &userID,     // user-id may be useless
                                    StaticGrid *sgrid, vector<LocalHGrid *> &multiGrids, Trip *trajectory, bool compressed) {

    long *statistics = new long[4]{};

    // ************************ Step-1: sort segments

    lSegment::SegQueue candidateSegs;

    GeoPoint *targetP = sgrid->getGeoPointById(targetPointId);

    searchSegmentsBottomUpDown(targetPointId, targetP, expectedIncrease, sgrid, multiGrids, candidateSegs, statistics, compressed);

    // ************************ Step-2: insertion time
    map<int, map<int, vector<lSegment *>>> granularity2zid2newSegments;

    unsigned int success = insertIntoSegments(targetPointId, targetP, expectedIncrease,
                                              multiGrids, candidateSegs, granularity2zid2newSegments, statistics, compressed);

    long startTimer = Utils::millisecond();

    //  ************************ Step-3: insert new segments into the proper grids
    for(const auto& pair: granularity2zid2newSegments) {
        int gIDX = IDX(pair.first);
        for(const auto& pair2: pair.second) {
            int zid = pair2.first;
            auto gc = multiGrids[gIDX]->getGridCellByZID(zid);
            if(compressed && !gc->isValid()) {
                gc->activateSelf();
            }
            for(auto seg: pair2.second) {   // all new segments located in this grid cell
                gc->insertSegment(seg);
            }
        }
    }
    statistics[3] += (Utils::millisecond() - startTimer);

    trajectory->increaseLength(success);    // update trajectory
    statistics[0] = expectedIncrease - (int) success;

    granularity2zid2newSegments.clear();

    return statistics;
}

void LocalEdit::buildGridStructure(StaticGrid *sgrid, vector<LocalHGrid *> &multiGrids) {

    int granularity = FINEST_GRANULARITY;

    // initialize the empty hierarchical grids
    stack<LocalHGrid *> tmp;
    while (granularity >= COARSEST_GRANULARITY) {
        tmp.push(new LocalHGrid(sgrid, granularity, false));
        granularity /= 2;
    }
    while (!tmp.empty()) {
        multiGrids.emplace_back(tmp.top());
        tmp.pop();
    }

    // construct hierarchical relation
    for(int l = 0; l < multiGrids.size() - 1; l++) {
        multiGrids[l]->connectWithLowerGrid(multiGrids[l + 1]);
    }
}

// scan the trajectory to construct the hierarchical grids
vector<unordered_set<int>> LocalEdit::buildGridContent(StaticGrid *sgrid, vector<LocalHGrid *> &multiGrids, const Trip *trip) {

    const int finestGIDX = IDX(FINEST_GRANULARITY);
    const auto finestG = multiGrids[finestGIDX];

    vector<unordered_set<int>> validGridCells;
    validGridCells.reserve(finestGIDX + 1);
    for (int i = 0; i <= finestGIDX; i++) {
        validGridCells.emplace_back(unordered_set<int>());
    }

    int *prevZ = nullptr, *currZ = nullptr;           // always store the finest
    GeoPoint *prevGP = nullptr, *currGP = nullptr;
    auto currP = trip->getFirstPoint();
    while (currP != nullptr) {
        if(currP->isTailPtr()) {
            finestG->insertSegment(prevZ[0], currP->getPrevPoint(), prevGP);    // the last point (segment)
            validGridCells[finestGIDX].insert(prevZ[0]);
            currZ = nullptr;
            currP = nullptr;
        }
        else {
            currGP = sgrid->getGeoPointById(currP->getPointId());
            currZ = finestG->locateGeoPoint(currGP);
            if (prevZ == nullptr) {        // the first point (segment)
                finestG->insertSegment(currZ[0], currP, currGP);
                validGridCells[finestGIDX].insert(currZ[0]);
            }
            else if (prevZ[0] == currZ[0]) {    // matched pair of points in this grid cell
                finestG->insertSegment(currZ[0], currP->getPrevPoint(), prevGP, currP, currGP);
                validGridCells[finestGIDX].insert(currZ[0]);
            }
            else {
                int gap = 0;
                int *lz = prevZ, *rz = currZ;
                while (lz != nullptr && rz != nullptr && lz[0] != rz[0]) {
                    if (FINEST_GRANULARITY / pow(2, ++gap) >= 1) {
                        lz = HierarGrid::upgradeZvalue(lz);
                        rz = HierarGrid::upgradeZvalue(rz);
                    }
                }
                if(lz != nullptr) {
                    int targetGIDX = IDX(FINEST_GRANULARITY / pow(2, gap));
                    multiGrids[targetGIDX]->insertSegment(lz[0], currP->getPrevPoint(), prevGP, currP, currGP);
                    validGridCells[targetGIDX].insert(lz[0]);
                }
            }

            prevZ = currZ;
            prevGP = currGP;
            currP = currP->getNextPoint();
        }
    }

    // clear stage
    if (prevZ != nullptr){
        delete[] prevZ;
        prevZ = nullptr;
    }
    if(currZ != nullptr) {
        delete[] currZ;
        currZ = nullptr;
    }

    if(prevGP != nullptr) {
        prevGP = nullptr;
    }
    if(currGP != nullptr) {
        currGP = nullptr;
    }

    return validGridCells;
}

// do a DFS search to compress the grid cells by connecting valid cells
void LocalEdit::compressGridCells(vector<LocalHGrid *> &multiGrids, vector<unordered_set<int>> &validGridCells){
    int granularity = FINEST_GRANULARITY;
    while (granularity > COARSEST_GRANULARITY) {
        int gidx = IDX(granularity);
        auto cells = validGridCells[gidx];
        for(int zid: cells) {
            auto candiCell = multiGrids[gidx]->getGridCellByZID(zid);
            string zStr = candiCell->getBaseFourStr();
            int g = gidx - 1;
            do {
                auto cells_up = validGridCells[g];
                zStr.pop_back();
                zStr = zStr.empty() ? "0" : zStr;
                int zParent = GridCell::transferBaseFourToTen(zStr);
                if(cells_up.find(zParent) != cells_up.end() || g == 0) {   // the root is enforced to exist
                    auto gParent = multiGrids[g]->getGridCellByZID(zParent);
                    gParent->addChild(candiCell, true);
                    candiCell->setParent(gParent, true);
                    break;
                }
            } while (g-- > 0);
        }

        granularity /= 2;       // level by level
    }
}