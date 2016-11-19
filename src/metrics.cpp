#include "metrics.h"
#include "cfcluster.h"

data_t dot(const DataPoint &lhs, const DataPoint &rhs)
{
    data_t result = 0.0;
    for (size_t i = 0; i < lhs.size(); ++i)
        result += lhs[i] * rhs[i];
    return result;
}

data_t getDistance(const CF_Cluster &cf1, const CF_Cluster &cf2, Distance distance)
{
    data_t dist = 0;

    if (cf1 == cf2)
        return dist;

    switch(distance)
    {
    case Distance::D0:
    {
        DataPoint diff = cf1.X0 - cf2.X0;
        dist = sqrt(dot(diff, diff));
        break;
    }
    case Distance::D1:
        for (size_t j = 0; j < cf1.X0.size(); ++j)
            dist += fabs(cf1.X0[j] - cf2.X0[j]);
        break;
    case Distance::D2:
        dist = sqrt((cf1.N * cf2.SS + cf2.N * cf1.SS - 2 * dot(cf1.LS, cf2.LS)) / (cf1.N * cf2.N));
        break;
    case Distance::D3:
    {
        DataPoint sumLS = cf1.LS + cf2.LS;
        long sumN = cf1.N + cf2.N;
        data_t sumSS = cf1.SS + cf2.SS;
        dist = sqrt(2 * (sumN) * (sumSS) - dot(sumLS, sumLS) / (sumN * (sumN - 1)));
        break;
    }
    }
    return dist;
}
