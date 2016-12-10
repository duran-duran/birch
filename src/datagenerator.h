#ifndef __DATA_GENERATOR_H__
#define __DATA_GENERATOR_H__

#include "common.h"
#include <random>

class DataGenerator
{
public:
    DataGenerator(size_t dim, data_t min, data_t max);
    virtual ~DataGenerator(){}
    virtual DataPoint getPoint() = 0;
protected:
    size_t dim;
    data_t min, max;
    std::mt19937 generator;
};

class UniformGenerator : public DataGenerator
{
public:
    UniformGenerator(size_t dim, data_t min, data_t max);
    virtual DataPoint getPoint() override;
private:
    std::uniform_real_distribution<data_t> distr;
};

class NormalGenerator : public DataGenerator
{
public:
    NormalGenerator(size_t dim, data_t min, data_t max, size_t coresCnt);
    virtual DataPoint getPoint() override;
private:
    std::vector<DataPoint> cores;
    std::normal_distribution<data_t> distr;
};

#endif // DATAGENERATOR_H
