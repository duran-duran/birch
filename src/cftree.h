#ifndef __CFTREE_H__
#define __CFTREE_H__

#include "common.h"
#include <list>

data_t euclideanDistance(const data_point &a, const data_point &b);

class CF_Subcluster
{
public:
    long N;
    data_point LS;
    data_t SS;

    data_point centroid;
    data_t radius, diameter;

    CF_Node* child;

    CF_Subcluster(int dim);
    bool add(const data_point& entry);
    data_t distanceTo(const data_point& entry);
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
    void insert(const data_point& entry);
    CF_Vector_it findClosest(const data_point& entry);
    std::pair<CF_Subcluster, CF_Subcluster> splitNode();
};

#endif // __CFTREE_H__
