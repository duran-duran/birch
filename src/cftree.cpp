#include "cftree.h"
#include "metrics.h"

CF_Node::CF_Node(data_t threshold, size_t branching, size_t *entriesCount) :
    threshold(threshold),
    bFactor(branching),
    counter(entriesCount),
    leaf(true),
    root(true),
    prevLeaf(nullptr),
    nextLeaf(nullptr)
{
}

CF_Node::CF_Node(data_t threshold, size_t branching, size_t *entriesCount, bool isLeaf, const CF_Vector &subclusters) :
    threshold(threshold),
    bFactor(branching),
    counter(entriesCount),
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
        if (counter) ++*counter;
        return;
    }

    closest->add(entry);
    if (leaf)
    {
        if (closest->D > threshold)
        {
            closest->remove(entry);
            subclusters.push_back(entry);
            if (counter) ++*counter;
        }
    }
    else
    {
        auto node = closest->child;
        node->insert(entry);
        if (node->getSubclusters().size() > bFactor)
        {
            auto newClusters = node->splitNode();
            delete node;
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

    auto poles = CF_Cluster::getTwoFarthest(subclusters);

    CF_Vector subclusters1, subclusters2;

    for (auto it = subclusters.begin(); it != subclusters.end(); ++it)
    {
        if (getDistance(*it, *poles.first) < getDistance(*it, *poles.second))
            subclusters1.push_back(*it);
        else
            subclusters2.push_back(*it);
    }

    CF_Node *node1 = new CF_Node(threshold, bFactor, counter, leaf, subclusters1),
            *node2 = new CF_Node(threshold, bFactor, counter, leaf, subclusters2);

    if (leaf)
    {
        if (prevLeaf)
        {
            node1->setPrevLeaf(prevLeaf);
            prevLeaf->setNextLeaf(node1);
        }

        node1->setNextLeaf(node2);
        node2->setPrevLeaf(node1);

        if (nextLeaf)
        {
            node2->setNextLeaf(nextLeaf);
            nextLeaf->setPrevLeaf(node2);
        }
    }

    CF_Cluster cluster1(node1),
               cluster2(node2);

    return CF_Vector{cluster1, cluster2};
}

void CF_Node::clear()
{
    if (!leaf)
    {
        for (auto& entry : subclusters)
        {
            entry.child->clear();
            delete entry.child;
        }
    }
    subclusters.clear();
}

const CF_Vector &CF_Node::getSubclusters()
{
    return subclusters;
}

bool CF_Node::isLeaf()
{
    return leaf;
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
