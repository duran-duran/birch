#ifndef __CFTREE_H__
#define __CFTREE_H__

#include "common.h"
#include <list>

data_t euclideanDistance(const DataPoint &a, const DataPoint &b);

class CF_Subcluster
{
public:
    long N;
    DataPoint LS;
    data_t SS;

    DataPoint centroid;
    data_t radius, diameter;

    CF_Node* child;

    CF_Subcluster(int dim);
    bool add(const DataPoint& entry);
    data_t distanceTo(const DataPoint& entry);
    data_t distanceTo(const CF_Subcluster& entry);
};

using CF_Vector = std::list<CF_Subcluster>;
using CF_Vector_it = CF_Vector::iterator;

class CF_Node
{
public:
    bool leaf;
    CF_Vector subclusters;

    CF_Node *prevLeaf, *nextLeaf;

    data_t threshold;
    int bFactor;

    CF_Node(data_t threshold, int branching);
    void insert(const DataPoint& entry);
    CF_Vector_it findClosest(const DataPoint& entry);
    std::pair<CF_Subcluster, CF_Subcluster> splitNode();
};

#endif // __CFTREE_H__
