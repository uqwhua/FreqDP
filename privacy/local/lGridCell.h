//
// Created by Fengmei Jin on 14/11/21.
//

#ifndef HIERARCHICALGRID_LGRIDCELL_H
#define HIERARCHICALGRID_LGRIDCELL_H

#include "../GridCell.h"
#include "lSegment.h"

class lGridCell: public GridCell{

    // key is the start-point-ID
    unordered_map<int, vector<lSegment*>> segments;  // all segments which can only be covered by this grid cell rather than any smaller one

public:

    lGridCell(int z, int g): GridCell(z, g) {};

    [[nodiscard]] bool isValid() const override;

    [[nodiscard]] unsigned int getSegmentNum(bool trajSeg) const override;

    void insertSegment(lSegment* seg);

    int checkSegmentsForPointInsertion(int targetPid, const GeoPoint *targetP, StaticGrid *sgrid,
                                       lSegment::SegQueue &queue, int *num);
    void removeSegment(lSegment *seg);

    bool removeSegments(set<lSegment *> &toBeRemoved);

    void clearSegments() override;

    void clearAll() override;
};


#endif //HIERARCHICALGRID_LGRIDCELL_H
