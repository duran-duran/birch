#ifndef __CF_TREE_BUILDER_H__
#define __CF_TREE_BUILDER_H__

#include "common.h"
#include "cftree.h"
#include "minsquare.h"

class CF_TreeBuilder
{
public:
    CF_TreeBuilder(long pointsCnt, int dimensions, size_t branching, data_t initThreshold, size_t maxEntries, size_t trackEach);
    ~CF_TreeBuilder();

    void addPointToTree(const DataPoint &point);
    void rebuildTree();

    CF_Node *getTree() const {return tree;}
    const CF_Cluster &getTreeCluster() const {return treeCluster;}
    CF_Vector getAllLeafEntries();
private:
    long count;
    int dim;
    size_t branching;
    size_t maxEntries, leafEntriesCount;
    size_t trackEach; //track linear regression models each "trackEach" incoming data points

    CF_Node *tree;
    CF_Cluster treeCluster;

    MinSquare<data_t> tFunc, rFunc;

    std::vector<CF_Node*> getAllLeafNodes();

    void trackLinRegression();
    data_t getMaxLeafEntryDiameter();
    data_t getMinNewThreshold();
    data_t getNewThreshold();
};

#endif //__CF_TREE_BUILDER_H__
