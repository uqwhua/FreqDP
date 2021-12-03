//
// Created by Fengmei Jin on 23/7/21.
//

#ifndef HIERARCHICALGRID_FILEOUTPUT_H
#define HIERARCHICALGRID_FILEOUTPUT_H

#include "LogFile.h"
#include "../spatial/Trip.h"

using namespace std;

class FileOutput{

    static bool outputTrip(const string& filename, StaticGrid *grid, Trip* trip){
        ofstream outfile;
        outfile.open(filename); //, ios_base::out);
        if (outfile.is_open()) {
            stringstream ss;
            auto ptr = trip->getFirstPoint();
            while (ptr != nullptr && !ptr->isTailPtr()){
                long timestamp = ptr->getTimestamp();
                ss << grid->getCoordinates(ptr->getPointId()) << "," << timestamp << "," << Utils::Stamp2Time(timestamp) << "\n";
                ptr = ptr->getNextPoint();
            }
            if(ss.good()){
                outfile << ss.str();
            }
            ss.clear();
            outfile.close();
            return true;
        }
        return false;
    }

public:

    static string outputTrips(const string& folder_prefix, StaticGrid *grid, const map<string, Trip*>& trips) {
        string msg;
        string file_prefix1 = folder_prefix + "/trip_";
        if(!LogFile::testOrCreateDir(file_prefix1)){
            msg.append("[ERROR] Cannot create a directory for the folder " + file_prefix1 + " !\n");
            return msg;
        }

        bool success;
        for(auto& pair: trips) {
            string userID = pair.first;
            Trip* trajectory = pair.second;

            // 1) output long trajectory
            string filename = file_prefix1 + userID + ".txt";
            success = outputTrip(filename, grid, trajectory);
        }
        if(!success){
            msg.append("[ERROR] Cannot output the long trajectories into " + file_prefix1 + " !");
        }
        if(msg.empty()) {
            msg.append("[INFO] successfully output the original trajectories/short trips to " + folder_prefix + " !");
        }
        return msg;
    }
};

#endif //HIERARCHICALGRID_FILEOUTPUT_H
