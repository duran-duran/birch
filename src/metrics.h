#ifndef __METRICS_H__
#define __METRICS_H__

#include "common.h"

enum class Distance
{
    D0,
    D1,
    D2,
    D3
};

class CF_Cluster;

data_t getDistance(const CF_Cluster& cf1, const CF_Cluster& cf2, Distance distance = Distance::D2);

data_t dot(const DataPoint& lhs, const DataPoint& rhs);

#endif // __METRICS_H__

