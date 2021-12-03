//
// Created by s4570405 on 23/06/2021.
//

#ifndef HIERARCHICALGRID_COMMONHEADER_H
#define HIERARCHICALGRID_COMMONHEADER_H

#include <fstream>
#include <sstream>
#include <iostream>

#include <stdexcept>
#include <unistd.h>

#include <ctime>
#include <cmath>
#include <algorithm>

#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <map>
#include <unordered_map>
#include <unordered_set>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MIN3(a, b, c) ((MIN(a, b) < (c)) ? (MIN(a, b)) : (c))
#define IDX(g)  (int) log2(g)

#define MAX_REPEAT_INSERTION 100
#define SAMPLING_RATE   180.0   // the sampling rate of T-Drive is around 3 minutes = 180 sec

#define FINEST_GRANULARITY 512
#define COARSEST_GRANULARITY 1

#endif //HIERARCHICALGRID_COMMONHEADER_H
