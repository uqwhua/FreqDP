//
// Created by s4570405 on 21/06/2021.
//

#include "GeoPoint.h"

int GeoPoint::getPointId() const {
    return pid;
}

float GeoPoint::getLongitude() const {
    return coordinates[0];
}

float GeoPoint::getLatitude() const {
    return coordinates[1];
}

float rad(float d){
    return d * M_PI / 180.0;
}

float GeoPoint::computeDistanceByCoors(float lng1, float lat1, float lng2, float lat2){
    float EARTH_RADIUS = 6378.137f; //km
    float a = rad(lng1) - rad(lng2);
    float radLat1 = rad(lat1);
    float radLat2 = rad(lat2);
    float b = radLat1 - radLat2;
    float dis = EARTH_RADIUS * 2 * asin(sqrt(pow(sin(b / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(a/2),2)));
    return round(dis * 100000.0f) / 100000;
}

float GeoPoint::computeDistance(float lng, float lat) const {
    if(this->getLongitude() == lng && this->getLatitude() == lat){
        return 0;
    }
    return computeDistanceByCoors(this->getLongitude(), this->getLatitude(), lng, lat);
}

float GeoPoint::computeDistance(const GeoPoint *point) const {
    if(this->getLongitude() == point->getLongitude() && this->getLatitude() == point->getLatitude()){
        return 0;
    }
    return computeDistanceByCoors(this->getLongitude(), this->getLatitude(), point->getLongitude(), point->getLatitude());
}

string GeoPoint::toString() const {
    string str(to_string(coordinates[0]));
    str.append(",");
    str.append(to_string(coordinates[1]));
    return str;
}

void GeoPoint::clearCoors() {
    delete[] coordinates;
    coordinates = nullptr;
}

float GeoPoint::computeDotProduct(GeoPoint point) {
    return this->coordinates[0] * point.coordinates[0] + this->coordinates[1] * point.coordinates[1];
}




