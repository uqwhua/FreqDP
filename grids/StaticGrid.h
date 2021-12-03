//
// Created by s4570405 on 21/06/2021.
//

#ifndef HIERARCHICALGRID_STATICGRID_H
#define HIERARCHICALGRID_STATICGRID_H

#include "Grid.h"

//#define ARRAY_LENGTH 296710     // maxVertexID + 1, depends on the road network file
#define ARRAY_LENGTH 183026
#define MAX_ITERATION 10

class StaticGrid : public Grid {

    GeoPoint **vertex_array;    // pointID -> its coordinates

    long *vertex_gridIdx;   // pointID -> the grid ID where it locates

    float **pairwiseDistance;

    unordered_map<long, unordered_set<int>> gridIdx_vertices;   // gridID -> inside points

    [[nodiscard]] float computeDistance(int id1, int id2) const;

public:

    StaticGrid();

    StaticGrid(StaticGrid *sgrid, float step);

    void addVertices(GeoPoint **vertexArray);

    int getNearestVertexId(float lng, float lat);    // used for a simple map-matching

    [[nodiscard]] float getPairwiseDist(int pid1, int pid2) const;

    GeoPoint *getGeoPointById(int pid);

    string getCoordinates(int pid);

    string printout() override;

    void clearAll() override;

    void allocateGridIdForVertex(bool needGrid2Vertices = true);
};


#endif //HIERARCHICALGRID_STATICGRID_H
