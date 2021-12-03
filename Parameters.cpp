//
// Created by s4570405 on 21/06/2021.
//

#include "Parameters.h"
#include "io/FileReader.h"

Parameters::Parameters() {
    RoadNetworkFile = "";
    inputFolder = "";
    outputFolder = "";
    outputTrips = false;
    data_total = 0;
    reduction = -1;

    edit_global = false;
    edit_local = false;
    global_local_composition = "";
}

char *getCurrDirectory() {
    char *currDirectory;
    if ((currDirectory = getcwd(nullptr, 0)) == nullptr) {
        perror("getcwd error");
    }
    else {
        printf("Current Working Directory = %s\n", currDirectory);
    }
    return currDirectory;
}

string Parameters::readParameters(const string &filename) {
    char *currDirectory = getCurrDirectory();

    string fullpath(currDirectory);
    fullpath.append("/");
    fullpath.append(filename);    // fixed
    auto *freader = new FileReader(fullpath);

    map<string, string> key2value;
    freader->readParameterFile(key2value, " = ");

    for (auto &pair: key2value) {
        if (pair.first == "RoadNetworkFile") {
            RoadNetworkFile = pair.second;
        }
        else if (pair.first == "data_FolderInput") {
            inputFolder = pair.second;
        }
        else if (pair.first == "data_FolderOutput") {
            outputFolder = pair.second;
        }
        else if (pair.first == "output_results") {
            outputTrips = (pair.second == "true");
        }
        else if (pair.first == "linking_reduction") {
            reduction = stoi(pair.second);
        }
        else if (pair.first == "data_total") {
            data_total = stoi(pair.second);
        }
        else if (pair.first == "privacy_epsilon") {
            vector<string> tmp;
            FileReader::split(pair.second, tmp, ",");
            for (const string &v: tmp) {
                epsilons.emplace_back(stod(v));
            }
        }
        else if (pair.first == "edit_local") {
            edit_local = (pair.second == "true");
        }
        else if (pair.first == "edit_global") {
            edit_global = (pair.second == "true");
        }
        else if (pair.first == "global_local_composition_order") {
            global_local_composition = pair.second;
        }
    }
    return printout(key2value);
}

string Parameters::printout(const map<string, string> &key2value) {
    string msg = "[Parameters]\n";
    map<string, string>::iterator itr;
    for (auto &pair: key2value) {
        msg.append(pair.first);
        msg.append(" = ");
        msg.append(pair.second);
        msg.append("\n");
    }
    msg.append("---------------------------------------------------------------------------\n");
    return msg;
}

string Parameters::getRoadNetworkFileName() {
    return RoadNetworkFile;
}

string Parameters::getInputFolder() {
    return inputFolder;
}

string Parameters::getOutputFolder() {
    return outputFolder;
}

bool Parameters::getLocalEdit() const {
    return edit_local;
}

bool Parameters::getGlobalEdit() const {
    return edit_global;
}

int Parameters::getObjectNum() const {
    return data_total;
}

int Parameters::getReduction() const {
    return reduction;
}

vector<float> Parameters::getPrivacyBudget() {
    return epsilons;
}

bool Parameters::needOutputTrips() const {
    return outputTrips;
}

string Parameters::getCompositionOrder() {
    return global_local_composition;
}

