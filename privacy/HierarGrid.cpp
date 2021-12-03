//
// Created by Fengmei Jin on 28/10/21.
//

#include "HierarGrid.h"

#include <utility>
#include "../Utils.h"

int HierarGrid::getZvalueByRowCol(int row, int col) {
    int z = 0;
    for (int i = 0; i < 31; i++) {      // 31 bit if signed int
        z = z | (row & 1 << i) << (i + 1) | (col & 1 << i) << i;
    }
    return z;
}

int* HierarGrid::getRowColFromZvalue(int zvalue) {
    int row = 0, col = 0;
    int n = 0, z = zvalue;
    bool rowTurn = false;        // the last bit is row (y-axis), then bit-interleaving
    while (z != 0) {
        int c = z % 2;
        if(rowTurn) {
            row += (int) pow(2, n++) * c;
        }
        else {
            col += (int) pow(2, n) * c;
        }
        z /= 2;
        rowTurn = !rowTurn;
    }
    return new int[3]{zvalue, row, col};
}

HierarGrid::HierarGrid(Grid *_grid, int _granularity) {
    granularity = _granularity;

    longitude_range = new float[2]{_grid->getLongitudeRange(0), _grid->getLongitudeRange(1)};
    latitude_range = new float[2]{_grid->getLatitudeRange(0), _grid->getLatitudeRange(1)};

    step = new float[2];
    step[0] = (latitude_range[1] - latitude_range[0]) / (float) granularity;    // determine the row (Y-axis)
    step[1] = (longitude_range[1] - longitude_range[0]) / (float) granularity;  // determine the column (X-axis)
}

int HierarGrid::getGranularity() const {
    return granularity;
}

bool HierarGrid::gridCover(const GeoPoint *point) {
    return !(point->getLongitude() < longitude_range[0] || point->getLongitude() > longitude_range[1] ||
             point->getLatitude() < latitude_range[0] || point->getLatitude() > latitude_range[1]);
}

bool HierarGrid::isInvalidID(int gridId, int _granularity) {
    return gridId < 0 || gridId >= _granularity * _granularity;
}

// somehow this is a static method
int *HierarGrid::locateGeoPoint(const GeoPoint *point, int _granularity) {
    if (point == nullptr || !gridCover(point)) { // out of range
        return nullptr;
    }

    const int g = _granularity < 1 ? this->granularity : _granularity;
    const float *s = g == this->granularity ? this->step
            : new float [2]{(latitude_range[1] - latitude_range[0]) / (float) g, (longitude_range[1] - longitude_range[0]) / (float) g};

    // find the grid cell where this point locates
    int row = (int) ((point->getLatitude() - latitude_range[0]) / s[0]);
    if (row == g) {
        row = row - 1;
    }

    int col = (int) ((point->getLongitude() - longitude_range[0]) / s[1]);
    if (col == g) {
        col = col - 1;
    }

    int z = getZvalueByRowCol(row, col);    // get z-value for current grid cell

    return isInvalidID(z, g) ? nullptr : new int[3]{z, row, col};
}

int *HierarGrid::upgradeZvalue(const int *lowerZ, int gap) {
    if (lowerZ == nullptr)
        return nullptr;

    int g = (int) pow(2, gap);
    int row = lowerZ[1] / g;
    int col = lowerZ[2] / g;
    int z = getZvalueByRowCol(row, col);

    return new int[3]{z, row, col};
}

int* HierarGrid::findBestFitGrid(Segment *seg, const int *lz, const int *rz){
    if(lz == nullptr)
        lz = locateGeoPoint(seg->getPoint(), FINEST_GRANULARITY);
    if(rz == nullptr)
        rz = locateGeoPoint(seg->getPoint(false), FINEST_GRANULARITY);

    if(lz == nullptr) {
        return new int[2] {FINEST_GRANULARITY, rz[0]};
    }
    else if(rz == nullptr) {
        return new int[2] {FINEST_GRANULARITY, lz[0]};
    }

    int gap = 0;
    while (lz != nullptr && rz!= nullptr && lz[0] != rz[0]) {
        if(FINEST_GRANULARITY / pow(2, ++gap) >= 1) {
            lz = upgradeZvalue(lz);
            rz = upgradeZvalue(rz);
        }
    }
    if (lz != nullptr && rz!= nullptr && lz[0] == rz[0]) {    //done
        return new int[2]{FINEST_GRANULARITY / (int) pow(2, gap), lz[0]};
    }

    return nullptr;
}

void HierarGrid::clearGrids() {
    delete[] step;
    step = nullptr;

    delete[] longitude_range;
    longitude_range = nullptr;

    delete[] latitude_range;
    latitude_range = nullptr;
}
