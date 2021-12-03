//
// Created by s4570405 on 21/06/2021.
//

#include "Trip.h"

Trip::Trip() {
    length = 0;
    head_ptr = new Point;   //(Point*) malloc(sizeof(Point));  // just pointer, no data
    tail_ptr = new Point;   // (Point*) malloc(sizeof(Point));
}

Trip::Trip(Trip *pTrip) {
    head_ptr = new Point;   //(Point*) malloc(sizeof(Point));  // just pointer, no data
    tail_ptr = new Point;   //(Point*) malloc(sizeof(Point));
    length = 0;

    Point *p = pTrip->getFirstPoint();
    Point *prevP = head_ptr;
    Point *point;
    while (p != nullptr && !p->isTailPtr()) {
        point = new Point(p);
        prevP->next_ptr = point;
        point->prev_ptr = prevP;
        prevP = point;
        p = p->next_ptr;
        length++;
    }
    point->next_ptr = tail_ptr;     // this is the last point
    tail_ptr->prev_ptr = point;
}

unsigned int Trip::getLength() const {
    return length;
}

void Trip::appendPoint(const long timestamp, const int vertexId) {
    auto *point = new Point(timestamp, vertexId);

    if (length == 0) {
        head_ptr->next_ptr = point;
        point->prev_ptr = head_ptr;
    }
    else {
        tail_ptr->prev_ptr->next_ptr = point;
        point->prev_ptr = tail_ptr->prev_ptr;
    }
    point->next_ptr = tail_ptr;
    tail_ptr->prev_ptr = point;
    length++;
}

Point *Trip::getFirstPoint() const {
    if (length > 0) {
        return head_ptr->getNextPoint();
    }
    return nullptr;
}

void Trip::clearAllPoints() {
    auto p = head_ptr;
    Point *toDelete;
    while (p != nullptr) {
        toDelete = p;
        p = p->getNextPoint();
        toDelete->clear();
        delete toDelete;
        toDelete = nullptr;
    }
}

void Trip::decreaseLength(int alreadyDelete) {
    length -= alreadyDelete;
}

void Trip::increaseLength(unsigned int increase) {
    length += increase;
}


