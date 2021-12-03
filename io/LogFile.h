//
// Created by Fengmei Jin on 9/7/21.
//

#ifndef HIERARCHICALGRID_LOGFILE_H
#define HIERARCHICALGRID_LOGFILE_H

#include "../commonheader.h"
#include "../Utils.h"
#include <cstdarg>
#include <sys/stat.h>

using namespace std;

class LogFile {

    string logFilepath;

public:

    static bool testOrCreateDir(const string &pathname);

    explicit LogFile(string filepath);

    void addContent(string &content, bool screenprint = true);

    void addContent(string &content, float parameter, bool screenprint = true);

    static string getLogFileName(const string &outputFolder, const string &parameters_str, bool enableLocal,
                                 bool enableLocal_NextGlobal, bool enableGlobal, bool enableGlobal_NextLocal);
};


#endif //HIERARCHICALGRID_LOGFILE_H
