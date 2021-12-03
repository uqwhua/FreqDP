//
// Created by Fengmei Jin on 28/10/21.
//

#include "GridCell.h"

#include <utility>
#include "EditOperation.h"

string GridCell::transferBaseTenToFour(int value, int radix) {
    string ans;
    do {
        int t = value % radix;
        if (t >= 0 && t <= 9)
            ans += t + '0';
        else
            ans += (t - 10 + 'a');
        value /= radix;
    } while (value != 0);
    reverse(ans.begin(), ans.end());
    return ans;
}

int GridCell::transferBaseFourToTen(string baseFourStr) {
    float value = 0;
    for (unsigned int i = 0, len = baseFourStr.length(); i < len; i++) {
        int v = baseFourStr.at(len - i - 1) - '0';
        value += v * pow(4, i);     // to base 10
    }
    return (int) value;
}

void GridCell::setRange(float lat_min, float lat_max, float lng_min, float lng_max) {
    latitude_range = new float[2]{lat_min, lat_max};
    longitude_range = new float[2]{lng_min, lng_max};
}

bool GridCell::isCover(const GeoPoint *point) {
    return !(point->getLongitude() < longitude_range[0] || point->getLongitude() > longitude_range[1] ||
             point->getLatitude() < latitude_range[0] || point->getLatitude() > latitude_range[1]);
}

// the minimum distance from the given point to this grid cell
float GridCell::computeMinDist(const GeoPoint *point) {
    if(isCover(point)) {
        return 0;
    }
    float dist = 0;
    float lng = point->getLongitude(), lat = point->getLatitude();
    float lngMIN = longitude_range[0], lngMAX = longitude_range[1];
    float latMIN = latitude_range[0], latMAX = latitude_range[1];
    if (lng >= lngMIN && lng <= lngMAX) {
        if (lat < latMIN)
            dist = GeoPoint::computeDistanceByCoors(lng, lat, lng, latMIN);
        else if (lat > latMAX)
            dist = GeoPoint::computeDistanceByCoors(lng, lat, lng, latMAX);
        // else, cover (point inside)
    } else if (lat >= latMIN && lat <= latMAX) {
        if (lng < lngMIN)
            dist = GeoPoint::computeDistanceByCoors(lng, lat, lngMIN, lat);
        else if (lng > lngMAX)
            dist = GeoPoint::computeDistanceByCoors(lng, lat, lngMAX, lat);
    } else {
        if (lat > latMAX) {
            if (lng < lngMIN)
                dist = GeoPoint::computeDistanceByCoors(lng, lat, lngMIN, latMAX);
            else if (lng > lngMAX)
                dist = GeoPoint::computeDistanceByCoors(lng, lat, lngMAX, latMAX);
        } else if (lat < latMIN) {
            if (lng < lngMIN)
                dist = GeoPoint::computeDistanceByCoors(lng, lat, lngMIN, latMIN);
            else if (lng > lngMAX)
                dist = GeoPoint::computeDistanceByCoors(lng, lat, lngMAX, latMIN);
        }
    }
    return dist;
}

void GridCell::setParent(GridCell* parent, bool valid) {
    if(valid)
        this->validParent = parent;
    else
        this->structuralParent = parent;
}

//void GridCell::reset_structural_children(vector<GridCell *> _children) {
//    structuralChildren = std::move(_children);
//}

void GridCell::addChild(GridCell* ptr, bool validChild) {
    if(validChild)
        validChildren.emplace_back(ptr);
    else
        structuralChildren.emplace_back(ptr);
}

vector<GridCell *> GridCell::getChildren(bool validChild) {
    return validChild ? validChildren : structuralChildren;
}

GridCell *GridCell::getParent(bool valid) {
    return valid ? validParent : structuralParent;
}

int GridCell::getGranularity() const {
    return granularity;
}

int GridCell::getID() const {
    return zvalue;
}

string GridCell::getBaseFourStr() {
    return zStr;
}

void GridCell::clearValidPointers(){
    validParent = nullptr;
    validChildren.clear();
}

void GridCell::clearAll() {

    structuralParent = nullptr;
    structuralChildren.clear();

    validParent = nullptr;
    validChildren.clear();

    delete[] longitude_range;
    longitude_range = nullptr;

    delete[] latitude_range;
    latitude_range = nullptr;
}

// only for valid pointers
void GridCell::unlinkSelf() {
    if(validParent != nullptr) {
        validParent->removeChild(this);
        validParent->addChildren(this->validChildren);
    }

    for(auto child: validChildren) {
        child->setParent(validParent, true);
    }
    validParent = nullptr;
    validChildren.clear();
}

void GridCell::removeChild(GridCell *child) {
    auto itr = validChildren.begin();
    while (itr != validChildren.end()) {
        if((*itr) == child) {
            validChildren.erase(itr);
            break;
        }
        itr++;
    }
}

void GridCell::addChildren(vector<GridCell *> _children) {
    validChildren.insert(validChildren.end(), _children.begin(), _children.end());
}

void GridCell::activateSelf() {
    auto ancestor = this->structuralParent;
    while (ancestor != nullptr && !ancestor->isValid()) {
        ancestor = ancestor->structuralParent;
    }
    this->validParent = ancestor;
    if(this->granularity < FINEST_GRANULARITY) {
        auto possibleChildren = ancestor->getChildren(true);    // old children
        for(auto child: possibleChildren) {
            if(child->granularity > this->granularity) {
                string cStr = child->getBaseFourStr();
                int gap = (int) log2(child->granularity / this->granularity);
                while (gap-- > 0) {
                    cStr.pop_back();
                }
                int cZ = transferBaseFourToTen(cStr);
                if(cZ == this->zvalue) {
                    ancestor->removeChild(child);   // not belong to him anymore
                    this->validChildren.emplace_back(child);    // get a new child
                    child->setParent(this, true);   // new parent
                }
            }
        }
    }
    if(ancestor != nullptr) {
        this->validParent->addChild(this, true);
    }
}

int* GridCell::checkStructuralPointers() {
    int *good = new int[2]{0,0};
    if((granularity == COARSEST_GRANULARITY && structuralParent != nullptr) || (granularity > COARSEST_GRANULARITY && structuralParent == nullptr)) {
        good[0] = 1;
    }

    if((granularity < FINEST_GRANULARITY && structuralChildren.size() != 4) || (granularity == FINEST_GRANULARITY && !structuralChildren.empty())){
        good[1] = (int) structuralChildren.size();
    }
    return good;
}


