#include <stdio.h>
#include <cstdlib>
#include <iostream>
#include <mpi.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    long count;
    int dim;
    double *data, *buff = nullptr;
    char *filename;
    FILE *pfile;

    if (rank == 0) {
        if (argc < 2) {
            std::cout << "Usage:" << std::endl;
            std::cout << "\tbirch <input_file> <output_file>" << std::endl;
            return 1;
        }

        filename = argv[1];

        pfile = fopen(filename, "rb");
        fread((void*)&count, sizeof(count), 1, pfile);
        fread((void*)&dim, sizeof(dim), 1, pfile);



        std::cout << "Reading " << count << " " << dim << "-dimensional data points" << std::endl;
    }

    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    std::cout << "Process " << rank << ": dim = " << dim << std::endl;

    int chunk, *chunks;
    if (rank == 0)
    {
        chunks = (int*) malloc(size * sizeof(int));
        for (int r = 0; r < size; ++r)
        {
            chunks[r] = (int) ((r + 1) * ((double) count / size)) -  (int) (r * ((double) count / size));
            std::cout << "Chunk " << r << ": " << chunks[r] << std::endl;
        }
    }

    MPI_Scatter(chunks, 1, MPI_INT,
                &chunk, 1, MPI_INT,
                0, MPI_COMM_WORLD);

    data = (double*) malloc(dim * chunk * sizeof(double));

    std::cout << "Process " << rank << ": I'll take " << chunk << " data points" << std::endl;
    if (rank == 0)
    {
        fread(data, dim * sizeof(double), chunk, pfile);
        for (int r = 1; r < size; ++r)
        {
            buff = (double*)malloc(dim * chunks[r] * sizeof(double));
            fread(buff, dim * sizeof(double), chunks[r], pfile);
            MPI_Send(buff, dim * chunks[r], MPI_DOUBLE, r, 0, MPI_COMM_WORLD);
            free(buff);
        }
//        free(buff);
        free(chunks);
        fclose(pfile);
    }
    else
    {
        MPI_Recv(data, dim * chunk, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for (int r = 0; r < size; ++r)
    {
        if (rank == r)
        {
            for (int k = 0; k < chunk; ++k) {
                std::cout << "Process " << rank << ": got data point:";
                for (int i = 0; i < dim; ++i) {
                    std::cout << " " << data[k * dim + i];
                }
                std::cout << std::endl;
            }
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
//        free(chunks);
//    {
//        free(chunks);
//        double *data = (double*) malloc(count * dim * sizeof(double));

//        for (long k = 0; k < count; ++k) {
//            std::cout << "Data point:";
//            for (int i = 0; i < dim; ++i) {
//                fread((void*)(data + k * dim + i), sizeof(double), 1, pfile);
//                std::cout << " " << data[k * dim + i];
//            }
//            std::cout << std::endl;
//        }

//        fclose(pfile);

//    }
//    free(chunks);
    free(data);

    MPI_Finalize();

    return 0;
}

