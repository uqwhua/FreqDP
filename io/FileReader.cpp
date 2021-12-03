//
// Created by s4570405 on 21/06/2021.
//

#include "FileReader.h"

// some general functions
void FileReader::split(const string &str, vector<string> &tokens, const string &delim) {
    tokens.clear();
    auto start = str.find_first_not_of(delim, 0);
    auto position = str.find_first_of(delim, start);
    while (position != string::npos || start != string::npos) {
        tokens.emplace_back(move(str.substr(start, position - start)));        // [start, position)
        // next token
        start = str.find_first_not_of(delim, position);
        position = str.find_first_of(delim, start);
    }
}

long timestr_timestamp(const string &datetime = "1970.01.01 00:00:00") {
    if (datetime.length() < 19) {
        cout << "invalid string - cant convert to timestamp\n";
    }

    struct tm tm{};
    tm.tm_year = atoi(datetime.substr(0, 4).c_str()) - 1900;
    tm.tm_mon = atoi(datetime.substr(5, 2).c_str()) - 1;
    tm.tm_mday = atoi(datetime.substr(8, 2).c_str());
    tm.tm_hour = atoi(datetime.substr(11, 2).c_str());
    tm.tm_min = atoi(datetime.substr(14, 2).c_str());
    tm.tm_sec = atoi(datetime.substr(17, 2).c_str());

    return mktime(&tm);
}

// constructor
FileReader::FileReader(const string &filename, const string &mode) {
    if(mode == "open") {
        fin.open(filename, ifstream::in);
        if (!fin.good()) {
            string msg("file not found: ");
            msg.append(filename);
            throw runtime_error(msg);
        }
    }
    else if(mode == "exist"){   // whether the folder exists
        if (access(filename.c_str(), 0) == -1){
//        if (_access(filename.c_str(), 0) == -1){
            string msg("folder does not exist: ");
            msg.append(filename);
            throw runtime_error(msg);
        }
    }
}

void FileReader::close() {
    if (fin.is_open()) {
        fin.close();
    }
}

float FileReader::readTdriveTraj(const string &foldername, const int &total, StaticGrid* grid,
                                 map<string, Trip *>& trajectories) {
    // only for T-Drive data
    const int startId = 1, maxId = 10357;
    const int timePos = 1, lngPos = 2, latPos = 3;
    const int minPointNum = 500;

    printf("[PROGRESS] ");
    fflush(stdout);

    int cnt = 0;
    int userId = startId;
    while (trajectories.size() < total && userId <= maxId) {
        string userId_str = to_string(userId++);
        string filename(foldername);
        filename.append(userId_str).append(".txt");
        ifstream ff(filename);
        if (!ff.good()) {
            continue;
        }

        string line;
        Trip *trajectory = new Trip();
        long preTimestamp = -1;
        while (getline(ff, line)) {     // parse one line indicating one point
            vector<string> tokens;
            split(line, tokens, ",");
            long curTimestamp = timestr_timestamp(tokens.at(timePos));
            if(preTimestamp < 0 || curTimestamp != preTimestamp){   // noise: some consecutive points have the same timestamp
                preTimestamp = curTimestamp;
                float lng = stof(tokens.at(lngPos));
                float lat = stof(tokens.at(latPos));
                int vid = grid->getNearestVertexId(lng, lat);
                if(vid >= 0){
                    trajectory->appendPoint(curTimestamp, vid);
                }
            }

        }
        if(trajectory->getLength() >= minPointNum){
//            trajectory->sortByTime();     // T-drive is already ordered by time
                                            // NOTE that if the data is not sorted by time, please do the sorting
            trajectories[userId_str] = trajectory;

            // print out progress
            if (total >= 10 && ++cnt % (total / 10) == 0) {
                printf("%d%%...", (int) (cnt * 100.0 / total));
                fflush(stdout);
            }
        }
        else{
            trajectory->clearAllPoints();
            delete trajectory;
            trajectory = nullptr;
        }
    }
    fin.close();
    printf("done!\n");

    if(trajectories.size() != total){
        printf("[ALERT] the trajectory number %lu from input is inconsistent with the required number %d\n", trajectories.size(), total);
    }

    unsigned int avgLength = 0;
    for(const auto& pair: trajectories){
        avgLength += pair.second->getLength();
    }
    return (float) avgLength / (float) trajectories.size();
}

unsigned int FileReader::readRoadNetworkFile(StaticGrid *staticGrid, float grid_size) {
    float lngMax = -200, lngMin = 200;
    float latMax = -100, latMin = 100;

    auto** vertex_point = new GeoPoint*[ARRAY_LENGTH];
    int newID = 0;
    string line;
    vector<string> id2data, lnglat;
    while (fin.good()) {
        getline(fin, line);
        split(line, id2data, ",");
        if (id2data.size() == 2) {
            split(id2data.at(1), lnglat);   // here is the default delim " "
            float lng = stof(lnglat.at(0)) / 10000000;
            float lat = stof(lnglat.at(1)) / 10000000;
            auto *point = new GeoPoint(newID, lng, lat);
            vertex_point[newID++] = point;

            // for grid construction
            lngMax = max(lng, lngMax);
            lngMin = min(lng, lngMin);
            latMax = max(lat, latMax);
            latMin = min(lat, latMin);
        }

        id2data.clear();
        lnglat.clear();
    }
    fin.close();

    staticGrid->initializeSpace(lngMin, lngMax, latMin, latMax, grid_size);    // default step = 0.001
    staticGrid->addVertices(vertex_point);

    return newID;
}

void FileReader::readParameterFile(map<string, string> &key2value, const string &delim) {
    string line, paramName, paramValue;
    while (fin.good()) {
        getline(fin, line);
        if (!line.empty() && line[0] != '#') {
            vector<string> vec;
            split(line, vec, delim);
            paramName = vec.at(0);
            paramValue = vec.at(1);
            key2value[paramName] = paramValue;  // to output all parameters
        }
    }
    fin.close();
}
