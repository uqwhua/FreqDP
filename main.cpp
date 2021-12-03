#include "Parameters.h"
#include "io/FileOutput.h"
#include "io/FileReader.h"
#include "privacy/local//LocalMechanism.h"
#include "privacy/global/GlobalMechanism.h"

using namespace std;

int main() {
    // get parameters from the file "config.properties"
    auto param = new Parameters();
    string msg = param->readParameters("config.properties");
    const string roadNetworkFile = param->getRoadNetworkFileName();
    const string inputFolder = param->getInputFolder();
    const string outputFolder = param->getOutputFolder();
    const bool outputTrips = param->needOutputTrips();      // whether we output the anonymized trips or not

    const int dataTotal = param->getObjectNum();    // how many trajectories will be processed in this program
    const int cutoff = param->getReduction();       // signature reduction, aka top-m signature points for each trajectory

    const vector<float> epsilons = param->getPrivacyBudget();       // the privacy budget for differential privacy
    const bool pureGlobal = param->getGlobalEdit();
    const bool pureLocal = param->getLocalEdit();
    const bool compositionGL = param->getCompositionOrder().find("GL") != string::npos;     // global -> local
    const bool compositionLG = param->getCompositionOrder().find("LG") != string::npos;     // local  -> global

    delete param;   param = nullptr;

    // prepare the log file
    const string parameters_str = "_D" + to_string(dataTotal) + "_R" + to_string(cutoff);
    const string logFilepath = LogFile::getLogFileName(outputFolder, parameters_str, pureLocal, compositionLG,
                                                       pureGlobal, compositionGL);
    auto *logger = new LogFile(logFilepath);
    logger->addContent(msg);

    // read road-network file and construct a static grid storing all anchor points on the road network
    auto *static_grid = new StaticGrid;
    {
        msg.append("[INFO] reading the road network file from: " + roadNetworkFile);
        logger->addContent(msg);

        auto *freader = new FileReader(roadNetworkFile);    // check the file good or not
        unsigned int anchorVertexNum = freader->readRoadNetworkFile(static_grid);
        freader->close();   delete freader;     freader = nullptr;

        msg.append(static_grid->printout());    // statistic info about the road network
        msg.append("\t # of anchor vertices = " + to_string(anchorVertexNum));
        logger->addContent(msg);
    }

    // read original data and do the map-matching based on the static grid
    map<string, Trip *> trajectories_ori;
    {
        msg.append("\n[INFO] reading/map-matching the raw trajectory data from " + inputFolder);
        logger->addContent(msg);

        auto *data_reader = new FileReader(inputFolder, "exist");
        float avgTrajLen = data_reader->readTdriveTraj(inputFolder, dataTotal, static_grid, trajectories_ori);
        data_reader->close();   delete data_reader;     data_reader = nullptr;

        msg = "\t avg length of " + to_string(trajectories_ori.size()) + " input trajectories = " + to_string((int) avgTrajLen);
        logger->addContent(msg);
    }

    map<string, Signature *> signatures_ori;
    map<string, FrequencyVec *> freqVec_ori;
    unordered_set<int> sigPoints_ori;
    auto *globalITF_ori = new FrequencyVec;     // ITF, inverse trajectory frequency w.r.t. points
    {
        msg.append("\n[INFO] constructing original signatures ...");
        logger->addContent(msg);
        float avgDimension = Functions::construction(trajectories_ori, signatures_ori, freqVec_ori, globalITF_ori);
        msg.append("\t avg # of distinct points in each signature = " + to_string((int) avgDimension));
        logger->addContent(msg);

        // the top-m signature point set
        sigPoints_ori = Functions::extractTopSignaturePoints(signatures_ori, cutoff);
        msg = "\t total # of distinct top-" + to_string(cutoff) + " signature points = " + to_string(sigPoints_ori.size()) + "\n";
        logger->addContent(msg);
    }

    // start protection
    if(pureLocal) {
        msg.append("\n-----------------------------------------------------------------------------\n");
        msg.append("--------------------------[ Local Mechanism Starts ]-------------------------\n");
        msg.append("-----------------------------------------------------------------------------\n");
        logger->addContent(msg);

        const string folder_prefix = outputFolder + "modify_L" + (compositionLG ? "G" : "") + "/trips_" + parameters_str;

        // can repeatedly execute for different epsilon
        for (float eps: epsilons) {
            msg.append("**********************************************************\n");
            msg.append("[INFO] privacy budget = " + to_string(eps));
            logger->addContent(msg);

            // index trajectories_cur (not the ori)
            map<string, Trip *> trajectories_cur = Utils::copy2Map(trajectories_ori);

            // each trajectory has its selected points to be perturbed
            map<string, vector<int>> targetTopSigPoints = LocalMechanism::getTargetSigPoints(signatures_ori, cutoff, sigPoints_ori);

            LocalMechanism::perform(logger, cutoff, eps, static_grid, trajectories_cur, signatures_ori, freqVec_ori, targetTopSigPoints);

            if (compositionLG) {    // continue to global mechanism if needed
                GlobalMechanism::perform(dataTotal, eps, cutoff, logger, static_grid, trajectories_cur);
            }

            if (outputTrips) {
                string subfolder = folder_prefix + "_EPS" + Utils::float2str(eps);
                string rtn2 = FileOutput::outputTrips(subfolder, static_grid, trajectories_cur);
                msg.append(rtn2);
                logger->addContent(msg);
            }

            targetTopSigPoints.clear();
            Utils::mapClear_Trip(trajectories_cur);
        }
    }

    // useless hereafter
    Utils::mapClear_Sig(signatures_ori);
    Utils::mapClear_Freq(freqVec_ori);

    if (pureGlobal) {
        msg.append("\n------------------------------------------------------------------------------\n");
        msg.append("--------------------------[ Global Mechanism Starts ]-------------------------\n");
        msg.append("------------------------------------------------------------------------------\n");
        logger->addContent(msg);

        const string folder_prefix = outputFolder + "modify_G" + (compositionGL ? "L" : "") + "/trips_" + parameters_str;

        for (float eps: epsilons) {
            msg.append("**********************************************************\n");
            msg.append("[INFO] privacy budget = " + to_string(eps));
            logger->addContent(msg);

            // make a copy as well
            map<string, Trip *> trajectories_cur = Utils::copy2Map(trajectories_ori);

            GlobalMechanism::perform(dataTotal, eps, cutoff, logger, static_grid, trajectories_cur, globalITF_ori, sigPoints_ori);

            if (compositionGL) {    // continue to local mechanism if needed
                LocalMechanism::perform(logger, cutoff, eps, static_grid, trajectories_cur);
            }

            if (outputTrips) {
                string subfolder = folder_prefix + "_EPS" + Utils::float2str(eps);
                string rtn2 = FileOutput::outputTrips(subfolder, static_grid, trajectories_cur);
                msg.append(rtn2);
                logger->addContent(msg);
            }

            Utils::mapClear_Trip(trajectories_cur);
        }
    }
    msg.append("\n---------------------------------------------------------------------------\n");
    msg.append("The program is done!!");
    logger->addContent(msg);

    // cleaning step
    Utils::mapClear_Trip(trajectories_ori);
    sigPoints_ori.clear();

    delete logger;  logger = nullptr;

    static_grid->clearAll();
    delete static_grid;     static_grid = nullptr;

    globalITF_ori->clearArray();
    delete globalITF_ori;   globalITF_ori = nullptr;

    return 0;
}