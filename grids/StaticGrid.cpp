//
// Created by s4570405 on 21/06/2021.
//

#include "StaticGrid.h"

StaticGrid::StaticGrid() {
    vertex_array = nullptr;
    vertex_gridIdx = new long[ARRAY_LENGTH];        //(long*) malloc((maxVertexID + 1) * sizeof(long));
    pairwiseDistance = new float* [ARRAY_LENGTH];
    for(int i = 0; i < ARRAY_LENGTH; i++){
        pairwiseDistance[i] = new float [ARRAY_LENGTH];
    }
}

StaticGrid::StaticGrid(StaticGrid *sgrid, float step) {
    bool sameStep = this->initializeSpace(sgrid, step);
    vertex_array = sgrid->vertex_array;
    pairwiseDistance = sgrid->pairwiseDistance;
    if(!sameStep) {
        vertex_gridIdx = new long[ARRAY_LENGTH];        //(long*) malloc((maxVertexID + 1) * sizeof(long));
        allocateGridIdForVertex(false);
    }
    else {
        vertex_gridIdx = sgrid->vertex_gridIdx;
    }
}

void StaticGrid::addVertices(GeoPoint** vertexArray) {
    vertex_array = vertexArray;
    allocateGridIdForVertex();
}

void StaticGrid::allocateGridIdForVertex(bool needGrid2Vertices) {
    for(int pid = 0; pid < ARRAY_LENGTH; pid++){
        if(vertex_array[pid] != nullptr){
            long gid = getGridIdByGeopoint(vertex_array[pid]);   // the grid cell where this point locates
            vertex_gridIdx[pid] = gid;

            if(needGrid2Vertices)
                gridIdx_vertices[gid].insert(pid);
        }
        else{
            vertex_gridIdx[pid] = -1;
        }
    }
}

// only used in trajectory calibration (simple map-matching)
int StaticGrid::getNearestVertexId(float lng, float lat) {
    int vertexID = -1, iteration = 0;
    long centerGridID = getGridIdByGeopoint(lng, lat);
    while (vertexID == -1 && iteration < MAX_ITERATION) {
        vector<long> gridIDs;
        getSurrounding(centerGridID, iteration++, gridIDs);
        float minDist = INFINITY;
        for (long gid: gridIDs) {
            auto pointset = gridIdx_vertices.find(gid);
            if (pointset != gridIdx_vertices.end()) {
                for (int vid: pointset->second) {
                    GeoPoint* vertex = vertex_array[vid];
                    if (vertex != nullptr) {
                        float distance = vertex->computeDistance(lng, lat);   // raw point and road intersection
                        if (minDist > distance) {
                            minDist = distance;
                            vertexID = vid;
                        }
                    }
                }
            }
        }
    }
    return vertexID;
}

float StaticGrid::computeDistance(int id1, int id2) const {
    auto point1 = vertex_array[id1];
    auto point2 = vertex_array[id2];
    if(point1 == point2 || point1 == nullptr || point2 == nullptr){
        return 0;
    }
    return point1->computeDistance(point2);
}

float StaticGrid::getPairwiseDist(int pid1, int pid2) const {
    if(pid1 == pid2){
        return 0;
    }
    int minID = MIN(pid1, pid2);
    int maxID = MAX(pid1, pid2);
    float dist = pairwiseDistance[minID][maxID];
    if(dist == 0) {
        dist = computeDistance(minID, maxID);
        pairwiseDistance[minID][maxID] = dist;
    }
    return dist;
}

string StaticGrid::getCoordinates(int pid) {
    if(pid < ARRAY_LENGTH) {
        return vertex_array[pid]->toString();
    }
    return "";
}

string StaticGrid::printout() {
    string msg = Grid::printout();  // space
    if(!gridIdx_vertices.empty())
        msg.append("[Static GRID] # of valid grid cells that contain vertices = " + to_string(gridIdx_vertices.size()) + "\n");
    return msg;
}

void StaticGrid::clearAll() {
    Grid::clearAll();
    if(vertex_array != nullptr && pairwiseDistance != nullptr) {
        for (int i = 0; i < ARRAY_LENGTH; i++) {
            delete [] pairwiseDistance[i];
            pairwiseDistance[i] = nullptr;

            vertex_array[i]->clearCoors();
            delete [] vertex_array[i];
            vertex_array[i] = nullptr;
        }
        delete[] vertex_array;
        vertex_array = nullptr;

        delete [] pairwiseDistance;
        pairwiseDistance = nullptr;
    }
    delete [] vertex_gridIdx;
    vertex_gridIdx = nullptr;

    gridIdx_vertices.clear();
}

GeoPoint *StaticGrid::getGeoPointById(int pid) {
    if(pid < ARRAY_LENGTH) {
        return vertex_array[pid];
    }
    return nullptr;
}

