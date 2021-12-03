//
// Created by Fengmei Jin on 14/11/21.
//

#ifndef HIERARCHICALGRID_GGRIDCELL_H
#define HIERARCHICALGRID_GGRIDCELL_H

#include "../GridCell.h"
#include "gSegment.h"

class gGridCell : public GridCell {

    unordered_map<int, vector<gSegment *>> segments;

public:

    gGridCell(int z, int g) : GridCell(z, g) {};

    [[nodiscard]] bool isValid() const override;

    [[nodiscard]] unsigned int getSegmentNum(bool trajSeg) const override;

    void clearSegments() override;

    void clearAll() override;

    void insertSegment(gSegment *seg);

    long checkSegmentsForInsertOnce(int targetPid, const GeoPoint *targetP, StaticGrid *sgrid,
                                    const unordered_set<string> &unavailableUsers, gSegment::SegUserQueue &queue,
                                    unordered_map<string, gSegment::SegUserNode *> &selectedUsers, int expectIncrease);

    void removeSegment(gSegment *toBeRemovedSeg);
};


#endif //HIERARCHICALGRID_GGRIDCELL_H
