//
// Created by Fengmei Jin on 9/7/21.
//

#include "LogFile.h"

bool LogFile::testOrCreateDir(const string& pathname){
    size_t found = pathname.find_last_of('/');
    string folder = pathname.substr(0, found + 1);
    ifstream fout (folder);
    if(!fout.good()){
        string commend = "mkdir -p " + folder;
        system(commend.c_str());
        fout.open(folder.c_str(), ios_base::in);
        return fout.good();
    }
    return true;
}

LogFile::LogFile(string filepath) {
    logFilepath = std::move(filepath);
    string msg;
    if(!testOrCreateDir(logFilepath)){
        msg = "[ERROR] Cannot create a directory for the log file " + logFilepath + " !\n";
    } else {
        msg = "[INFO] The log file is " + logFilepath + " !\n";
    }
    addContent(msg);
}

void LogFile::addContent(string& content, bool screenprint) {
    ofstream outfile;
    outfile.open(logFilepath, ios_base::app | ios_base::out);
    if (outfile.is_open()) {
        outfile << content << endl;
        outfile.close();
    }
    else {
        cout << "[ERROR] Cannot open the log file " << logFilepath << " !\n";
    }
    if(screenprint){
        printf("%s\n", content.c_str());
    }
    content.clear();
}

string LogFile::getLogFileName(const string &outputFolder, const string &parameters_str, bool enableLocal,
                               bool enableLocal_NextGlobal, bool enableGlobal, bool enableGlobal_NextLocal) {
    string logFilepath = outputFolder + "logs/log_" + parameters_str;
    if (enableLocal) {
        logFilepath.append("_L");
        if (enableLocal_NextGlobal)
            logFilepath.append("G");
    }
    if (enableGlobal) {
        logFilepath.append("_G");
        if (enableGlobal_NextLocal)
            logFilepath.append("L");
    }
    logFilepath.append("_T" + to_string(Utils::millisecond()));     // timestamp
    logFilepath.append(".csv");
    return logFilepath;
}