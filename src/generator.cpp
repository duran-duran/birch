#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>

int main(int argc, char *argv[])
{
    if (argc < 5) {
        printf("Usage:\n\tgenerator <count> <dimensions> <min_value> <max_value> <filename>\n");
        return 1;
    }

    long count = atoi(argv[1]);

    int dim   = atoi(argv[2]);

    double min = atof(argv[3]),
           max = atof(argv[4]);

    char *filename = argv[5];

    double *data = (double*) malloc(count * dim * sizeof(double));

    std::cout << "Generating " << count << " " << dim << "-dimensional data points" << std::endl;
    for (long k = 0; k < count; ++k) {
        std::cout << "Data point:";
        for (int i = 0; i < dim; ++i) {
            data[k * dim + i] = (double)rand() / RAND_MAX  * max + min;
            std::cout << " " << data[k * dim + i];
        }
        std::cout << std::endl;
    }

    FILE *pfile = fopen(filename, "wb");

    fwrite(&count, sizeof(count), 1, pfile);
    fwrite(&dim, sizeof(dim), 1, pfile);
    fwrite(data, sizeof(double), count * dim, pfile);

    fclose(pfile);

    return 0;
}
