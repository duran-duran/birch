#include "cftree.h"
#include <limits>
#include <cmath>
#include <utility>

CF_Node::CF_Node(data_t threshold, int branching) :
    threshold(threshold),
    bFactor(branching),
    leaf(true),
    prevLeaf(nullptr),
    nextLeaf(nullptr)
{

}

void CF_Node::insert(const DataPoint &entry)
{
    if (!leaf)
    {
        auto closest = findClosest(entry);
        closest.add(entry);
        closest.child->insert(entry);
    }
    else
    {
        if (subclusters.empty() || !findClosest(entry).add(entry))
            auto newSubcluster = CF_Subcluster(entry.size());
    }

    if (subclusters.size() > bFactor)
        splitNode();
}

CF_Vector_it CF_Node::findClosest(const DataPoint &entry)
{
    auto closest = subclusters.begin();

    data_t shortestDist;

    if (closest != subclusters.end())
    {
        shortestDist = closest->distanceTo(entry);
        for (auto it = closest + 1; it != subclusters.end(); ++it)
        {
            auto distance = it->distanceTo(entry);
            if (distance < shortestDist)
            {
                closest = it;
                shortestDist = distance;
            }
        }
    }

    return closest;
}

std::pair<CF_Vector_it, CF_Vector_it> CF_Node::splitNode()
{
    data_t longestDist = 0;
    CF_Subcluster firstPole, secondPole;

    for (auto lhs = subclusters.begin(); lhs != subclusters.end() - 1; ++lhs)
    {
        for (auto rhs = lhs + 1; rhs != subclusters.end(); ++rhs)
        {
            auto distance = lhs->distanceTo(*rhs);
            if (distance > longestDist)
            {
                longestDist = distance;
                firstPole = *lhs;
                secondPole = *rhs;
            }
        }
    }

    CF_Vector firstPoleClusters, secondPoleClusters;
    firstPoleClusters.push_back(firstPole);
    secondPoleClusters.push_back(secondPole);
    subclusters.erase(firstPole);
    subclusters.erase(secondPole);

    for (auto it = subclusters.begin(); it != subclusters.end(); ++it)
    {
        if (firstPole.distanceTo(*it) < secondPole.distanceTo(*it))
            firstPoleClusters.push_back(*it);
        else
            secondPoleClusters.push_back(*it);
    }

    return std::make_pair(
                CF_Subcluster()
                )
    //find two farthest subclasters
}

CF_Subcluster::CF_Subcluster(int dim) :
    N(0), LS(0, dim), SS(0),
    centroid(0, dim),
    radius(0), diameter(0),
    child(nullptr)
{
}

data_t CF_Subcluster::distanceTo(const DataPoint &entry)
{
    return euclideanDistance(entry, centroid);
}

data_t CF_Subcluster::distanceTo(const CF_Subcluster &entry)
{
    return entry.distanceTo(centroid);
}

data_t euclideanDistance(const DataPoint &a, const DataPoint &b)
{
    data_t sum = 0;
    for (size_t i = 0; i < a.size(); ++i)
        sum += pow(a[i] - b[i], 2);
    return sqrt(sum);
}

