#ifndef __CFTREE_H__
#define __CFTREE_H__

#include "common.h"
#include "cfcluster.h"

class CF_Node
{
public:
    CF_Node(data_t threshold, size_t branching, size_t *entriesCount = nullptr);
    CF_Node(data_t threshold, size_t branching, size_t *entriesCount, bool isLeaf, const CF_Vector& subclusters);

    const CF_Vector& getSubclusters();

    bool isLeaf();

    void setPrevLeaf(CF_Node *leaf);
    void setNextLeaf(CF_Node *leaf);

    CF_Node *getPrevLeaf();
    CF_Node *getNextLeaf();

    void insert(const CF_Cluster& entry);
    CF_Vector splitNode();

    void clear();

private:
    data_t threshold;
    size_t bFactor;
    size_t *counter;

    bool leaf, root;
    CF_Node *prevLeaf, *nextLeaf;

    CF_Vector subclusters;
};

#endif // __CFTREE_H__
