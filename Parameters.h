//
// Created by s4570405 on 21/06/2021.
//

#ifndef HIERARCHICALGRID_PARAMETERS_H
#define HIERARCHICALGRID_PARAMETERS_H

#include "commonheader.h"

using namespace std;

class Parameters {
    // data parameters
    string RoadNetworkFile, inputFolder, outputFolder;
    bool outputTrips;
    int data_total;
    int reduction;

    // privacy parameters
    vector<float> epsilons;
    bool edit_global;
    bool edit_local;
    string global_local_composition;

    static string printout(const map<string, string>& key2value);

public:

    Parameters();

    string readParameters(const string& filename);

    string getRoadNetworkFileName();

    string getInputFolder();

    string getOutputFolder();

    [[nodiscard]] bool needOutputTrips() const;

    [[nodiscard]] int getObjectNum() const;

    [[nodiscard]] int getReduction() const;

    vector<float> getPrivacyBudget();

    [[nodiscard]] bool getGlobalEdit() const;

    [[nodiscard]] bool getLocalEdit() const;

    string getCompositionOrder();       // GL, LG, GLLG
};


#endif //HIERARCHICALGRID_PARAMETERS_H
