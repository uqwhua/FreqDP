//
// Created by s4570405 on 24/06/2021.
//

#include "GlobalEdit.h"
#include "../../Utils.h"

void GlobalEdit::modify(FrequencyVec *globalITF_ori, FrequencyVec *globalITF_pert, StaticGrid *sgrid,
                        map<string, Trip *> &trajectories, long *timeCosts) {


    long startTimer = Utils::millisecond();
    auto finestGrid = constructFinestGrid(FINEST_GRANULARITY, sgrid, trajectories);
    long constructionTime = Utils::millisecond() - startTimer;

    map<int, int> point2expectedIncrease;

    printf("[PROGRESS-Deletion ] ");
    fflush(stdout);
    long decreaseTime = 0;

    for (int pid = 0; pid < ARRAY_LENGTH; pid++) {
        int oriITF = globalITF_ori->getValueByKey(pid);
        if (oriITF > 0) {   // those points whose oriITF are zero indicate no trajectory passed through it, invalid points

            int pertITF = globalITF_pert->getValueByKey(pid);
            int diff = pertITF - oriITF;
            if (diff != 0) {

                if (diff < 0) {
                    startTimer = Utils::millisecond();
                    singlePointDeletion(pid, oriITF, diff, sgrid, finestGrid, trajectories);
                    decreaseTime += (Utils::millisecond() - startTimer);
                }
                else {  // for increasing points, process them later
                    point2expectedIncrease[pid] = diff;
                }
            }
        }

        // print out progress
        if (pid % (ARRAY_LENGTH / 10) == 0) {
            printf("%d%%...", (int) (pid * 100.0 / ARRAY_LENGTH));
            fflush(stdout);
        }
    }
    printf("done!\n");   // deletion is done
    fflush(stdout);

    finestGrid->clearUselessInfo();
    finestGrid->clearGrids();
    finestGrid = nullptr;

    startTimer = Utils::millisecond();

    // to store who contains a certain point (then they are not available)
    vector<unordered_set<string>> point2users;
    point2users.reserve(ARRAY_LENGTH);
    for(int i = 0; i < ARRAY_LENGTH; i++)
        point2users.emplace_back(unordered_set<string>());  // empty initialize

    vector<GlobalHGrid*> multiGrids;
    buildGridStructure(sgrid, multiGrids);
    for(const auto& tripPair: trajectories) {
        buildGridContent(sgrid, multiGrids, tripPair.first, tripPair.second, point2users);
    }
    constructionTime += Utils::millisecond() - startTimer;

    // heuristic: deletion --> insertion
    printf("[PROGRESS-Insertion] ");
    fflush(stdout);

    long increaseTime = 0, sortTime = 0, insertTime = 0, updateTime = 0;
    unsigned int c = 0, totalIncreasePoint = point2expectedIncrease.size();
    for(auto point: point2expectedIncrease) {
        startTimer = Utils::millisecond();
        int pid = point.first;
        auto *rtn = singlePointInsertion(pid, point.second, sgrid, multiGrids, point2users[pid], trajectories);
        increaseTime += (Utils::millisecond() - startTimer);
        sortTime += rtn[1];
        insertTime += rtn[2];
        updateTime += rtn[3];

        // print out progress
        if (++c % (totalIncreasePoint / 10) == 0) {
            printf("%d%%...", (int) (c * 100.0 / totalIncreasePoint));
            fflush(stdout);
        }

        delete[] rtn;
        rtn = nullptr;
    }
    printf("done!\n");   // deletion is done
    fflush(stdout);

    timeCosts[2] = decreaseTime;
    timeCosts[3] = increaseTime;
    timeCosts[4] = sortTime;
    timeCosts[5] = insertTime;
    timeCosts[6] = updateTime;
    timeCosts[7] = constructionTime;

    point2users.clear();
    point2users.shrink_to_fit();
    point2expectedIncrease.clear();
    Utils::clearGrids(multiGrids);
}


/** reduce the occurrence of the target point, to satisfy the perturbed ITF value
 * @param expectedDecrease a negative value
 * */
int GlobalEdit::singlePointDeletion(int targetPointId, int oriITF, int expectedDecrease,
                                    StaticGrid *sgrid, GlobalHGrid *finestGrid, map<string, Trip *> &trajectories) {
    int *rtn = finestGrid->locateGeoPoint(sgrid->getGeoPointById(targetPointId));
    int targetGridId = rtn[0];

    set<string> selectedUsers;
    if (oriITF + expectedDecrease > 0) {
        int decreaseNum = abs(expectedDecrease);
        vector<pair<string, float>> sortedCost;
        finestGrid->selectTopUsersForGlobalPointDeletion(targetGridId, targetPointId, decreaseNum, sgrid, sortedCost);     // user, cost
        for (const auto &pair: sortedCost) {
            selectedUsers.insert(pair.first);
            if (selectedUsers.size() == decreaseNum) {
                break;
            }
        }
        sortedCost.clear();
    }

    map<string, int> user2DeletedNum = finestGrid->completeDeletePointFromGrid(targetPointId, targetGridId,selectedUsers);
    int remaining = expectedDecrease + (int) user2DeletedNum.size();
    for (const auto &pair: user2DeletedNum) {
        Trip *trajectory = trajectories[pair.first];
        trajectory->decreaseLength(pair.second);

#ifdef DEBUG_GLOBAL
        string uid = pair.first;
        int *rtn2 = trajectory->checkDeletedPoint(targetPointId);
        if (rtn2[0] != -1) {
            printf("[ERROR] GlobalEdit.cpp -> reduceOccurrence() for user %s, inconsistent trip length! traj.length = %d, check = %d\n",
                   uid.c_str(), trajectory->getLength(), rtn2[0]);
        }
        if (rtn2[1] != -1) {
            printf("[ERROR] GlobalEdit.cpp -> reduceOccurrence() for user %s, point %d: expected_freq = 0, remaining = %d!\n",
                   uid.c_str(), targetPointId, rtn2[1]);
        }
#endif
    }
    user2DeletedNum.clear();

    return remaining;
}

int insertIntoSelectedSegments(int targetPid, int expectedIncrease, GeoPoint *targetP, vector<GlobalHGrid *> &multiGrids,
                               map<string, Trip *> &trajectories, gSegment::SegUserQueue &candidateSegs, long *&statistics) {
    int success = 0;
    map<int, map<int, vector<pair<string, gSegment*>>>> gIDX2zid2newSegs;
    auto finestG = multiGrids.back();
    auto targetZ_finest = finestG->locateGeoPoint(targetP);

    long startTimer = Utils::millisecond(), updateTimeTotal = 0, updateTimer;
    while (!candidateSegs.empty() && success < expectedIncrease) {
        auto candi = candidateSegs.top();
        candidateSegs.pop();

        string uid = candi->_uid;

        auto targetSegNode = candi->topOneSegNode();    // with the minimum cost for this user
        auto targetSeg = targetSegNode->_seg;
        auto newSegs = targetSeg->split_insert(targetPid, targetP, uid);
        trajectories[uid]->increaseLength(1);
        success++;

        // work for grid update -- remove some segments
        updateTimer = Utils::millisecond();
        if(targetSeg->empty()) {
            int gIDX = IDX(targetSegNode->_granularity), zid = targetSegNode->_zid;
            multiGrids[gIDX]->getGridCellByZID(zid)->removeSegment(targetSeg);
        }

        // insert these two new segments into a proper level (grid cell)
        auto leftSeg = get<0>(newSegs);
        auto lz = finestG->findBestFitGrid(leftSeg, nullptr, targetZ_finest);
        if(lz != nullptr) {
            int g = IDX(lz[0]), z = lz[1];
            gIDX2zid2newSegs[g][z].emplace_back(make_pair(uid, leftSeg));
        }

        auto rightSeg = get<1>(newSegs);
        auto rz = finestG->findBestFitGrid(rightSeg, targetZ_finest, nullptr);
        if(rz != nullptr) {
            int g = IDX(lz[0]), z = lz[1];
            gIDX2zid2newSegs[g][z].emplace_back(make_pair(uid, rightSeg));
        }

        updateTimeTotal += (Utils::millisecond() - updateTimer);
    }
    statistics[2] = Utils::millisecond() - startTimer - updateTimeTotal;  // insert time

    delete[] targetZ_finest;
    targetZ_finest = nullptr;
    finestG = nullptr;

    // update: insert new segments
    updateTimer = Utils::millisecond();
    for(const auto& eachGranularity: gIDX2zid2newSegs) {
        auto hg = multiGrids[eachGranularity.first];
        for(const auto& eachCell: eachGranularity.second) {
            int zid = eachCell.first;
            for(const auto& segPair: eachCell.second) {
                hg->getGridCellByZID(zid)->insertSegment(segPair.second);
            }
        }
    }
    updateTimeTotal += (Utils::millisecond() - updateTimer);
    statistics[3] = updateTimeTotal;
    gIDX2zid2newSegs.clear();

    return success;
}

long searchSegmentsBottomUpDown(int targetPointId, int expectIncrease, const GeoPoint *targetP, StaticGrid *sgrid, const vector<GlobalHGrid *> &multiGrids,
                              const unordered_set<string> &unavailUsers, gSegment::SegUserQueue &candidateSegs){
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
    candidateGrids_bottomup.push(HierarGrid::QNode(startCell, 0));

    float kDist = INFINITY;    // top-k segment distance, initially infinite
    bool rootChecked = false;

    long checkedCellNum = 0, prunedCellNum = 0, checkedSegNum = 0;

    // start search by first bottom-up then top-down
    gGridCell *candiGC;
    float mindist;
    unordered_map<string, gSegment::SegUserNode *> selectedUsers;
    while (!candidateGrids_bottomup.empty() || !candidateGrids_topdown.empty()) {
        if(!rootChecked) {
            auto candi = candidateGrids_bottomup.top();
            candidateGrids_bottomup.pop();

            candiGC = dynamic_cast<gGridCell *>(candi._gc);
            mindist = candi._minDist;
        }
        else {
            auto candi = candidateGrids_topdown.top();
            candidateGrids_topdown.pop();
            candiGC = dynamic_cast<gGridCell *>(candi._gc);

            // early terminate after root
            if (candi._minDist > kDist && candidateSegs.size() >= expectIncrease) {     // hereafter, cannot exist promising segments
                break;
            }
        }

        visited[IDX(candiGC->getGranularity())][candiGC->getID()] = true;

        // if already found k-segments but the root hasn't been accessed
        if (!rootChecked && candidateSegs.size() >= expectIncrease){
            mindist = mindist >= 0 ? mindist : candiGC->computeMinDist(targetP);
            if (mindist > kDist){     // the current candidate cannot contain any promising segments
                prunedCellNum++;
                continue;
            }
        }

        checkedCellNum++;
        if(candiGC->isValid()) {    // own some segments inside
            checkedSegNum += candiGC->checkSegmentsForInsertOnce(targetPointId, targetP, sgrid, unavailUsers, candidateSegs, selectedUsers, expectIncrease);
            if(candidateSegs.size() >= expectIncrease) {
                kDist = candidateSegs.top()->minimumDist();      // only after k-segments are found, this threshold will work
            }
        }

        // first push structuralParent during bottom-up
        if(!rootChecked) {
            auto gParent = candiGC->getParent();
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
        auto children = candiGC->getChildren();
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
    long sortTime = Utils::millisecond() - startTimer;      // sort time

    for(auto pair: visited) {
        delete[] pair;
        pair = nullptr;
    }
    visited.clear();

    for(auto pair: selectedUsers) {
        pair.second->clearAll();
        pair.second = nullptr;
    }
    selectedUsers.clear();
    targetP = nullptr;

    return sortTime;
}

long *GlobalEdit::singlePointInsertion(int targetPointId, int expectedIncrease, StaticGrid *sgrid, vector<GlobalHGrid *> &multiGrids,
                                       unordered_set<string> &unavailUsers, map<string, Trip *> &trajectories) {
    long *statistics = new long[4]{};

    GeoPoint *targetP = sgrid->getGeoPointById(targetPointId);

    // search top segments
    gSegment::SegUserQueue candidateSegs;
    statistics[1] = searchSegmentsBottomUpDown(targetPointId, expectedIncrease, targetP, sgrid, multiGrids, unavailUsers, candidateSegs);     // return total sort time


    // insert the point into selected segments (insert time)
    // and update grids (update time)
    int success = insertIntoSelectedSegments(targetPointId, expectedIncrease, targetP,
                                             multiGrids, trajectories, candidateSegs, statistics);
    statistics[0] = expectedIncrease - success;

    return statistics;
}

GlobalHGrid *GlobalEdit::constructFinestGrid(int granularity, StaticGrid *sgrid, const map<string, Trip *> &trajectories) {

    auto finestGrid = new GlobalHGrid(sgrid, granularity, true);

    for(const auto& tripPair: trajectories) {
        string uid = tripPair.first;
        auto traj = tripPair.second;

        // currently, just point-based information
        finestGrid->computeZgridForPoints(uid, traj, sgrid);
    }
    return finestGrid;
}

void GlobalEdit::buildGridStructure(StaticGrid *sgrid, vector<GlobalHGrid *> &multiGrids) {
    int granularity = FINEST_GRANULARITY;

    // initialize the empty hierarchical grids
    stack<GlobalHGrid *> tmp;
    while (granularity >= COARSEST_GRANULARITY) {
        tmp.push(new GlobalHGrid(sgrid, granularity, false));
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

void GlobalEdit::buildGridContent(StaticGrid *sgrid, vector<GlobalHGrid*> &multiGrids, const string& uid, Trip *trip, vector<unordered_set<string>> &point2users) {
    // scan the trajectory to construct the hierarchical grids
    auto currP = trip->getFirstPoint();
    auto finestG = multiGrids[IDX(FINEST_GRANULARITY)];
    int* prevZ = nullptr, *currZ = nullptr;           // always store the finest
    GeoPoint *prevGP = nullptr, *currGP = nullptr;

    while (currP != nullptr) {
        if(currP->isTailPtr()) {
            finestG->insertSegment(uid, prevZ[0], currP->getPrevPoint(), prevGP);
            currZ = nullptr;
            currP = nullptr;
        }
        else {
            int pid = currP->getPointId();
            point2users[pid].insert(uid);
            currGP = sgrid->getGeoPointById(pid);
            currZ = finestG->locateGeoPoint(currGP);
            if(prevZ == nullptr) {
                finestG->insertSegment(uid, currZ[0], currP, currGP);
            }
            else if(prevZ[0] == currZ[0]) {
                finestG->insertSegment(uid, currZ[0], currP->getPrevPoint(), prevGP, currP, currGP);
            }
            else {
                int gap = 0;
                int *lz = prevZ, *rz = currZ;
                while (lz != nullptr && rz != nullptr && lz[0] != rz[0]) {
                    if(FINEST_GRANULARITY / pow(2, ++gap) >= 1) {
                        lz = HierarGrid::upgradeZvalue(lz);
                        rz = HierarGrid::upgradeZvalue(rz);
                    }
                }
                if(lz != nullptr) {
                    int targetG = IDX(FINEST_GRANULARITY / pow(2, gap));
                    multiGrids[targetG]->insertSegment(uid, lz[0], currP->getPrevPoint(), prevGP, currP, currGP);
                }
            }

            prevZ = currZ;
            prevGP = currGP;
            currP = currP->getNextPoint();
        }
    }

    if(prevZ != nullptr) {
        delete[] prevZ;
        prevZ = nullptr;
    }
    if(currZ != nullptr) {
        delete[] currZ;
        currZ = nullptr;
    }
}