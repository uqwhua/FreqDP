//
// Created by s4570405 on 21/06/2021.
//

#ifndef HIERARCHICALGRID_FILEREADER_H
#define HIERARCHICALGRID_FILEREADER_H

#include "../spatial/Trip.h"
#include "../grids/StaticGrid.h"

using namespace std;

class FileReader {

    ifstream fin;

public:
    explicit FileReader(const string& filename, const string& mode = "open");

    void close();

    static void split(const string &str, vector<string> &tokens, const string &delim = " ");

    unsigned int readRoadNetworkFile(StaticGrid *staticGrid, float grid_size = 0.01);

    void readParameterFile(map<string, string>& key2value, const string& delim);

    float readTdriveTraj(const string &foldername, const int &total, StaticGrid* grid, map<string, Trip *>& trajectories);
};


#endif //HIERARCHICALGRID_FILEREADER_H
