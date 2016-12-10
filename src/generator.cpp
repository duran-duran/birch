#include <stdlib.h>

#include "common.h"
#include "datagenerator.h"

int rank, procs; //to resolve externs from common.h

int main(int argc, char *argv[])
{
    if (argc < 5) {
        LOG("Usage:");
        LOG("\tgenerator <count> <dimensions> <min_value> <max_value> <pattern> <filename>");
        LOG("\t<pattern> = {uniform, normal}");
        return 1;
    }

    long count = atoi(argv[1]);

    int dim = atoi(argv[2]);

    data_t min = atof(argv[3]),
           max = atof(argv[4]);

    std::string pattern(argv[5]);

    char *filename = argv[6];

    DataGenerator* generator;
    if (pattern == "uniform")
        generator = new UniformGenerator(dim, min, max);
    else if (pattern == "normal")
        generator = new NormalGenerator(dim, min, max, std::log(count));

    LOG("Generating %d %d-dimensional data points", count, dim);

    FILE *pfile = fopen(filename, "wb");

    fwrite(&count, sizeof(count), 1, pfile);
    fwrite(&dim, sizeof(dim), 1, pfile);

    for (long i = 0; i < count; ++i)
    {
        DataPoint point = generator->getPoint();
        LOG("Data point: %s", pointToString(point).c_str());
        fwrite(&point[0], sizeof(data_t), dim, pfile);
    }

    fclose(pfile);
    delete generator;

    return 0;
}
