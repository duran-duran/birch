#include "cftreebuilder.h"
#include "metrics.h"

CF_TreeBuilder::CF_TreeBuilder(long pointsCnt, int dimensions, size_t branching, data_t initThreshold, size_t maxEntries, size_t trackEach) :
    count(pointsCnt), dim(dimensions),
    branching(branching),
    maxEntries(maxEntries), leafEntriesCount(0),
    trackEach(trackEach),
    tree(new CF_Node(initThreshold, branching, &leafEntriesCount)),
    treeCluster(tree)
{
}

CF_TreeBuilder::~CF_TreeBuilder()
{
    if (tree)
    {
        tree->clear();
        delete tree;
    }
}

void CF_TreeBuilder::addPointToTree(const DataPoint &point)
{
    tree->insert(CF_Cluster(point));
    treeCluster.add(CF_Cluster(point));

    if (treeCluster.N % trackEach == 0)
        trackLinRegression();
    if (leafEntriesCount > maxEntries)
        rebuildTree();
}

void CF_TreeBuilder::rebuildTree()
{
    auto subclusters = getAllLeafEntries();
    data_t newT = getNewThreshold();

    tree->clear();
    delete tree;

    leafEntriesCount = 0;

    tree = new CF_Node(newT, branching, &leafEntriesCount);
    for (const auto& entry : subclusters)
        tree->insert(entry);
    treeCluster = CF_Cluster(tree);
}

std::vector<CF_Node*> CF_TreeBuilder::getAllLeafNodes()
{
    std::vector<CF_Node*> result;
    auto node = tree;

    while (!node->isLeaf())
        node = node->getSubclusters().front().child;
    result.push_back(node);
    for (CF_Node *left = node->getPrevLeaf(); left != nullptr; left = left->getPrevLeaf())
        result.push_back(left);
    for (CF_Node *right = node->getNextLeaf(); right != nullptr; right = right->getNextLeaf())
        result.push_back(right);
    return result;
}

CF_Vector CF_TreeBuilder::getAllLeafEntries()
{
    CF_Vector result;
    auto leafNodes = getAllLeafNodes();
    for (auto node : leafNodes)
        result.insert(result.end(), node->getSubclusters().begin(), node->getSubclusters().end());
    return result;
}

void CF_TreeBuilder::trackLinRegression()
{
    rFunc.addPoint(std::make_pair(treeCluster.N, treeCluster.R));
    tFunc.addPoint(std::make_pair(treeCluster.N, getMaxLeafEntryDiameter()));
}

data_t CF_TreeBuilder::getMaxLeafEntryDiameter()
{
    auto leafEntries = getAllLeafEntries();
    data_t max = 0;
    for (const auto& entry : leafEntries)
    {
        if (entry.D > max)
            max = entry.D;
    }
    return max;
}

data_t CF_TreeBuilder::getMinNewThreshold()
{
    auto node = tree;
    while (!node->isLeaf())
    {
        long maxN = 0;
        for (const auto& entry : node->getSubclusters())
        {
            if (entry.N > maxN)
                node = entry.child;
        }
    }

    auto leafEntries = node->getSubclusters();

    if (leafEntries.size() < 2)
        return 0;

    auto twoClosest = CF_Cluster::getTwoClosest(leafEntries);

    return getDistance(*twoClosest.first, *twoClosest.second);
}

data_t CF_TreeBuilder::getNewThreshold()
{
    auto newN = std::min(2 * treeCluster.N, count);

    auto expF = std::max(1.0, rFunc.getY(newN) / rFunc.getY(treeCluster.N));
    auto newT = tFunc.getY(newN);
    newT = std::max(getMinNewThreshold(), expF * newT);
    if (newT < tFunc.getY(treeCluster.N))
        newT *= std::pow(newN / treeCluster.N, 1 / dim);
    return newT;
}

