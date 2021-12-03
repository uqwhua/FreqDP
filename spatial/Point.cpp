//
// Created by Fengmei Jin on 5/7/21.
//

#include "Point.h"

Point::Point(Point *pPoint) {
    pointPair = make_pair(pPoint->getTimestamp(), pPoint->getPointId());
    prev_ptr = nullptr;
    next_ptr = nullptr;
}

long Point::getTimestamp() const {
    return pointPair.first;
}

int Point::getPointId() const {
    return pointPair.second;
}

bool Point::isHeadPtr() const{
    return prev_ptr == nullptr;     // the head pointer of a trip
}

bool Point::isTailPtr() const{
    return next_ptr == nullptr;     // the tail pointer of a trip
}

Point* Point::getPrevPoint() const {
    return prev_ptr;
}

Point* Point::getNextPoint() const {
    return next_ptr;
}

[[maybe_unused]] bool Point::compareByTime(Point* a, Point* b) {
    return a->pointPair.first < b->pointPair.first;
}

void Point::clear() {
    prev_ptr = nullptr;
    next_ptr = nullptr;
}

// insert this point before the curP
[[maybe_unused]] void Point::insertBefore(Point *curP) {
    this->prev_ptr = curP->prev_ptr;
    this->next_ptr = curP;
    curP->prev_ptr->next_ptr = this;
    curP->prev_ptr = this;
}

// insert this point after the curP
void Point::insertAfter(Point *curP) {
    this->prev_ptr = curP;
    this->next_ptr = curP->next_ptr;
    curP->next_ptr->prev_ptr = this;
    curP->next_ptr = this;
}

void Point::unlink() const {
    this->prev_ptr->next_ptr = this->next_ptr;
    this->next_ptr->prev_ptr = this->prev_ptr;
}
