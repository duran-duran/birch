#include "datagenerator.h"
#include <chrono>

DataGenerator::DataGenerator(size_t dim, data_t min, data_t max) :
    dim(dim),
    min(min), max(max)
{
    generator.seed(std::chrono::system_clock::now().time_since_epoch().count());
}

UniformGenerator::UniformGenerator(size_t dim, data_t min, data_t max) :
    DataGenerator(dim, min, max),
    distr(min, max)
{
}

DataPoint UniformGenerator::getPoint()
{
    DataPoint result(dim);
    for (size_t i = 0; i < dim; ++i)
        result[i] = distr(generator);
    return result;
}

NormalGenerator::NormalGenerator(size_t dim, data_t min, data_t max, size_t coresCnt) :
    DataGenerator(dim, min, max),
    cores(coresCnt, DataPoint((data_t)0, dim)),
    distr(0.0, (max - min) / coresCnt)
{
    std::uniform_real_distribution<data_t> coresDistr(min, max);
    for (size_t i = 0; i < coresCnt; ++i)
    {
        for (size_t j = 0; j < dim; ++j)
            cores[i][j] = coresDistr(generator);
    }
}

DataPoint NormalGenerator::getPoint()
{
    size_t coreInd = generator() % cores.size();
    DataPoint result = cores[coreInd];

    for (size_t i = 0; i < dim; ++i)
        result[i] += distr(generator);
    return result;
}
