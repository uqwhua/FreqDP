//
// Created by s4570405 on 21/06/2021.
//

#ifndef HIERARCHICALGRID_TRIP_H
#define HIERARCHICALGRID_TRIP_H

#include <list>
#include "Point.h"

// a linked list
class Trip {

    Point* head_ptr;    // just pointer, no data
    Point* tail_ptr;
    unsigned int length;     // exclude the head/tail

public:

    explicit Trip();

    explicit Trip(Trip *pTrip);

    void appendPoint(long time, int pid);

    [[nodiscard]] unsigned int getLength() const;

    [[nodiscard]] Point * getFirstPoint() const;

    void clearAllPoints();

    void decreaseLength(int alreadyDelete);

    void increaseLength(unsigned int increase);
};


#endif //HIERARCHICALGRID_TRIP_H
