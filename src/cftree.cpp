#include "cftree.h"
#include "metrics.h"

CF_Node::CF_Node(data_t threshold, size_t branching) :
    threshold(threshold),
    bFactor(branching),
    leaf(true),
    root(true),
    prevLeaf(nullptr),
    nextLeaf(nullptr)
{
}

CF_Node::CF_Node(data_t threshold, size_t branching, bool isLeaf, const CF_Vector &subclusters) :
    threshold(threshold),
    bFactor(branching),
    leaf(isLeaf),
    root(false),
    prevLeaf(nullptr),
    nextLeaf(nullptr),
    subclusters(subclusters)
{
}

void CF_Node::insert(const CF_Cluster &entry)
{
    auto closest = entry.findClosest(subclusters);
    if (closest == subclusters.end())
    {
        subclusters.push_back(entry);
        return;
    }

    closest->add(entry);
    if (leaf && closest->D > threshold)
    {
        closest->remove(entry);
        subclusters.push_back(entry);
    }
    else
    {
        auto node = closest->child;
        node->insert(entry);
        if (node->getSubclusters().size() > bFactor)
        {
            auto newClusters = node->splitNode();
            subclusters.erase(closest);
            subclusters.insert(subclusters.end(), newClusters.begin(), newClusters.end());
        }
    }

    if (root && subclusters.size() > bFactor)
    {
        subclusters = splitNode();
        leaf = false;
    }
}

CF_Vector CF_Node::splitNode()
{
    if (subclusters.size() < 2)
        return (root) ? subclusters : CF_Vector{CF_Cluster(this)};

    CF_Vector_it pole1 = subclusters.begin(),
                 pole2 = pole1 + 1;
    data_t longestDist = getDistance(*pole1, *pole2);

    for (auto lhs = pole1; lhs != subclusters.end() - 1; ++lhs)
    {
        for (auto rhs = pole2 + 1; rhs != subclusters.end(); ++rhs)
        {
            auto distance = getDistance(*lhs, *rhs);
            if (distance > longestDist)
            {
                longestDist = distance;
                pole1 = lhs;
                pole2 = rhs;
            }
        }
    }

    CF_Vector subclusters1{*pole1},
              subclusters2{*pole2};

    for (auto it = subclusters.begin(); it != subclusters.end(); ++it)
    {
        if (it != pole1 && it != pole2)
        {
            if (getDistance(*it, *pole1) < getDistance(*it, *pole2))
                subclusters1.push_back(*it);
            else
                subclusters2.push_back(*it);
        }
    }

    CF_Node *node1 = new CF_Node(threshold, bFactor, leaf, subclusters1),
            *node2 = new CF_Node(threshold, bFactor, leaf, subclusters2);

    if (leaf)
    {
        node1->setPrevLeaf(prevLeaf);
        node1->setNextLeaf(node2);

        node2->setPrevLeaf(node1);
        node2->setNextLeaf(nextLeaf);
    }

    CF_Cluster cluster1(node1),
               cluster2(node2);

    return CF_Vector{cluster1, cluster2};
}

const CF_Vector &CF_Node::getSubclusters()
{
    return subclusters;
}

void CF_Node::setPrevLeaf(CF_Node *leaf)
{
    prevLeaf = leaf;
}

void CF_Node::setNextLeaf(CF_Node *leaf)
{
    nextLeaf = leaf;
}

CF_Node *CF_Node::getPrevLeaf()
{
    return prevLeaf;
}

CF_Node *CF_Node::getNextLeaf()
{
    return nextLeaf;
}


