//
// Created by s4570405 on 21/06/2021.
//

#include "Grid.h"

void Grid::initializeSpace(float lng_min, float lng_max, float lat_min, float lat_max, float _step){

    step = _step;

    // set a larger spatial rectangle
    longitude_range = new float[2] {floor(lng_min)-1, ceil(lng_max)+1};
    latitude_range = new float[2] {floor(lat_min)-1, ceil(lat_max)+1};

    float lng_diff = longitude_range[1] - longitude_range[0];  // max - min
    float lat_diff = latitude_range[1] - latitude_range[0];
    numOfCellX_col = (long) ceil(lng_diff / step);
    numOfCellY_row = (long) ceil(lat_diff / step);
}

bool Grid::initializeSpace(Grid* grid, float _step) {

    step = _step;   // step depends on the given parameter

    // range is the same
    longitude_range = grid->longitude_range;
    latitude_range = grid->latitude_range;

    // numOfCell may be different due to the step
    float lng_diff = longitude_range[1] - longitude_range[0];  // max - min
    float lat_diff = latitude_range[1] - latitude_range[0];
    numOfCellX_col = (long) ceil(lng_diff / step);
    numOfCellY_row = (long) ceil(lat_diff / step);

    return _step == grid->step;
}

long Grid::getGridIdByGeopoint(float lng, float lat){
    if(!gridCover(lng, lat)){
        return -1;
    }

    long idx_row = (long) floor((latitude_range[1] - lat) / step);
    long idx_col = (long) floor((lng - longitude_range[0]) / step);
    long grid_id = idx_row * numOfCellX_col + idx_col;

    if(is_invalid(grid_id)){
        return -1;  // should not happen
    }
    return grid_id;
}

long Grid::getGridIdByGeopoint(GeoPoint *point){
    return getGridIdByGeopoint(point->getLongitude(), point->getLatitude());
}

bool Grid::is_invalid(long gridId) const{
    return gridId < 0 || gridId >= numOfCellX_col * numOfCellY_row;
}

bool Grid::gridCover(float lng, float lat){
    return !(lng < longitude_range[0] || lng > longitude_range[1] ||
             lat < latitude_range[0] || lat > latitude_range[1]);
}

bool Grid::getSurrounding(long centerGridID, int iteration, vector<long> &results) const {
    if(iteration == 0){
        results.push_back(centerGridID);
    }
    else if(iteration == 1){
        getSurrounding(centerGridID, results);
    }
    else if(iteration > 1){
        long row = floor((float)centerGridID / (float)numOfCellX_col);
        long col = centerGridID - row * numOfCellX_col;
        long rMin = MAX(row - iteration, 0), rMax = MIN(row + iteration, numOfCellY_row);
        long cMin = MAX(col - iteration, 0), cMax = MIN(col + iteration, numOfCellX_col);

        // up and down
        for(long c = cMin; c <= cMax; c++){
            long gid = rMin * numOfCellX_col + c;
            if(!is_invalid(gid)){
                results.push_back(gid);
            }
            gid = rMax * numOfCellX_col + c;
            if(!is_invalid(gid)){
                results.push_back(gid);
            }
        }
        // left and right
        for(long r = rMin + 1; r < rMax; r++){
            long gid = r * numOfCellX_col + cMin;
            if(!is_invalid(gid)){
                results.push_back(gid);
            }
            gid = r * numOfCellX_col + cMax;
            if(!is_invalid(gid)){
                results.push_back(gid);
            }
        }
        return rMin == 0 || rMax == numOfCellY_row || cMax == 0 || cMax == numOfCellX_col;  // the boundary
    }
    return false;
}

void Grid::getSurrounding(long centerGridID, vector<long> &results) const {
    if(!is_invalid(centerGridID - 1)){  // left
        results.push_back(centerGridID - 1);
    }
    if(!is_invalid(centerGridID + 1)){  // right
        results.push_back(centerGridID + 1);
    }
    if(!is_invalid(centerGridID - numOfCellX_col)){ // up
        results.push_back(centerGridID - numOfCellX_col);
    }
    if(!is_invalid(centerGridID - numOfCellX_col - 1)){
        results.push_back(centerGridID - numOfCellX_col - 1);
    }
    if(!is_invalid(centerGridID - numOfCellX_col + 1)){
        results.push_back(centerGridID - numOfCellX_col + 1);
    }
    if(!is_invalid(centerGridID + numOfCellX_col)){ // down
        results.push_back(centerGridID + numOfCellX_col);
    }
    if(!is_invalid(centerGridID + numOfCellX_col - 1)){
        results.push_back(centerGridID + numOfCellX_col - 1);
    }
    if(!is_invalid(centerGridID + numOfCellX_col + 1)){
        results.push_back(centerGridID + numOfCellX_col + 1);
    }
}

string Grid::printout() {
    float up = GeoPoint::computeDistanceByCoors(longitude_range[0], latitude_range[1], longitude_range[1], latitude_range[1]);
    float down = GeoPoint::computeDistanceByCoors(longitude_range[0], latitude_range[0], longitude_range[1], latitude_range[0]);
    float left = GeoPoint::computeDistanceByCoors(longitude_range[0], latitude_range[0], longitude_range[0], latitude_range[1]);
    float right = GeoPoint::computeDistanceByCoors(longitude_range[1], latitude_range[0], longitude_range[1], latitude_range[1]);
    float horizontal_length = (up + down) / (2 * (float) numOfCellX_col);
    float vertical_length = (left + right) / (2 * (float) numOfCellY_row);
    char buffer[200];
    sprintf(buffer, "[Spatial GRID] longitude = [%.1f, %.1f], latitude = [%.1f, %.1f], step = %.3f, cells = %ld * %ld, size of cell = %.3f * %.3f\n",
            longitude_range[0], longitude_range[1], latitude_range[0], latitude_range[1], step, numOfCellX_col, numOfCellY_row, horizontal_length, vertical_length);
    return buffer;
}

void Grid::clearAll(){
    delete[] longitude_range;
    longitude_range = nullptr;

    delete[] latitude_range;
    latitude_range = nullptr;
}
