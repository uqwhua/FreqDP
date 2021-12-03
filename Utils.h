//
// Created by Fengmei Jin on 5/7/21.
//

#ifndef HIERARCHICALGRID_UTILS_H
#define HIERARCHICALGRID_UTILS_H

#include <sys/timeb.h>

#include "spatial/Trip.h"
#include "linking/Signature.h"
#include "linking/FrequencyVec.h"
#include "privacy/local/LocalHGrid.h"
#include "privacy/global/GlobalHGrid.h"

class Utils {
public:
    static long long millisecond() {
        timeb t;
        ftime(&t);
        return t.time * 1000 + t.millitm;
    }

    static string Stamp2Time(long timestamp) {
        time_t tick = (time_t) timestamp;
        struct tm tm;
        char s[40];
        tm = *localtime(&tick);
        strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);
        string str(s);
        return str;
    }

    static map<string, Trip *> copy2Map(const map<string, Trip *> &trajectories) {
        map<string, Trip *> trajectories_cp;
        for (auto &pair: trajectories) {
            trajectories_cp[pair.first] = new Trip(pair.second);
        }
        return trajectories_cp;
    }

    static void clearGrids(vector<LocalHGrid *> &vec, bool completely = true) {
        for (auto hg: vec) {
            if (completely) {
                hg->clearGrids();
                delete hg;
                hg = nullptr;
            } else {
                hg->clearContent();
            }
        }
        if (completely) {
            vec.clear();
            vec.shrink_to_fit();
        }
    }

    static void clearGrids(vector<GlobalHGrid *> &vec) {
        for (auto hg: vec) {
            hg->clearGrids();
            delete hg;
            hg = nullptr;
        }
        vec.clear();
        vec.shrink_to_fit();
    }

    static void mapClear_Sig(map<string, Signature *> &map) {
        auto itr = map.begin();
        while (itr != map.end()) {
            (*itr).second->clearMap();
            delete (*itr).second;
            (*itr).second = nullptr;
            itr = map.erase(itr);
        }
        map.clear();
    }

    static void mapClear_Freq(map<string, FrequencyVec *> &map) {
        auto itr = map.begin();
        while (itr != map.end()) {
            (*itr).second->clearArray();
            delete (*itr).second;
            (*itr).second = nullptr;
            itr = map.erase(itr);
        }
        map.clear();
    }

    static void mapClear_Trip(map<string, Trip *> &map) {
        auto itr = map.begin();
        while (itr != map.end()) {
            (*itr).second->clearAllPoints();
            delete (*itr).second;
            (*itr).second = nullptr;
            itr = map.erase(itr);
        }
        map.clear();
    }

    // from 0.01 to 0-01
    static string float2str(float value) {
        string str = to_string(value);
        size_t found = str.find_last_of('.');
        str.replace(found, 1, "-");
        found = str.find_last_not_of('0');
        if (str.at(found) == '-') {
            return str.substr(0, found + 2);
        } else {
            return str.substr(0, found + 1);
        }
    }
};

#endif //HIERARCHICALGRID_UTILS_H
