#ifndef __CFTREE_H__
#define __CFTREE_H__

#include "common.h"
#include "cfcluster.h"

class CF_Node
{
public:
    CF_Node(data_t threshold, size_t branching);
    CF_Node(data_t threshold, size_t branching, bool isLeaf, const CF_Vector& subclusters);

    const CF_Vector& getSubclusters();

    void setPrevLeaf(CF_Node *leaf);
    void setNextLeaf(CF_Node *leaf);

    CF_Node *getPrevLeaf();
    CF_Node *getNextLeaf();

    void insert(const CF_Cluster& entry);
    CF_Vector splitNode();

private:
    data_t threshold;
    size_t bFactor;

    bool leaf, root;
    CF_Node *prevLeaf, *nextLeaf;

    CF_Vector subclusters;
};

#endif // __CFTREE_H__
