//
// Created by Fengmei Jin on 5/7/21.
//

#ifndef HIERARCHICALGRID_POINT_H
#define HIERARCHICALGRID_POINT_H

#include <utility>

using namespace std;

// node of the douly linked list
class Point {
    pair <long, int> pointPair;     // time, pointID

public:
    Point* prev_ptr;
    Point* next_ptr;

    // constructor
    Point(): pointPair(make_pair(-1, -1)), prev_ptr(nullptr), next_ptr(nullptr){};  // head or tail

    Point(long t, int id): pointPair(make_pair(t, id)), prev_ptr(nullptr), next_ptr(nullptr){};

    explicit Point(Point *pPoint);

    void clear();

    [[nodiscard]] int getPointId() const;

    [[nodiscard]] long getTimestamp() const;

    [[nodiscard]] bool isHeadPtr() const;

    [[nodiscard]] bool isTailPtr() const;

    [[nodiscard]] Point *getPrevPoint() const;

    [[nodiscard]] Point *getNextPoint() const;

    [[maybe_unused]] static bool compareByTime(Point *a, Point *b);

    [[maybe_unused]] void insertBefore(Point *curP);

    void insertAfter(Point *pPoint);

    void unlink() const;
};


#endif //HIERARCHICALGRID_POINT_H
