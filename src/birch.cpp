#include "common.h"



int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    long count;
    int dim;
    data_t *data, *buff = nullptr;
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
        fread(&count, sizeof(count), 1, pfile);
        fread(&dim, sizeof(dim), 1, pfile);



        std::cout << "Reading " << count << " " << dim << "-dimensional data points" << std::endl;
    }

    MPI_Bcast(&dim, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Datatype DATA_POINT;
    MPI_Type_contiguous(dim, MPI_DATA_T, &DATA_POINT);
    MPI_Type_commit(&DATA_POINT);
    const int point_size = sizeof(data_t) * dim;

    std::cout << "Process " << rank << ": dim = " << dim << std::endl;

    int chunk, *chunks;
    if (rank == 0)
    {
        chunks = (int*) malloc(procs * sizeof(int));
        for (int r = 0; r < procs; ++r)
        {
            chunks[r] = (int) ((r + 1) * ((double) count / procs)) -  (int) (r * ((double) count / procs));
            std::cout << "Chunk " << r << ": " << chunks[r] << std::endl;
        }
    }

    MPI_Scatter(chunks, 1, MPI_INT,
                &chunk, 1, MPI_INT,
                0, MPI_COMM_WORLD);

    data = (data_t*) malloc(chunk * point_size);

    std::cout << "Process " << rank << ": I'll take " << chunk << " data points" << std::endl;
    if (rank == 0)
    {
        fread(data, point_size, chunk, pfile);
        for (int r = 1; r < procs; ++r)
        {
            buff = (data_t*)realloc(buff, chunks[r] * point_size);
            fread(buff, point_size, chunks[r], pfile);
            MPI_Send(buff, chunks[r], DATA_POINT, r, 0, MPI_COMM_WORLD);
//            free(buff);
        }
//        free(buff);
        free(chunks);
        fclose(pfile);
    }
    else
    {
        MPI_Recv(data, chunk, DATA_POINT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    for (int r = 0; r < procs; ++r)
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

    MPI_Type_free(&DATA_POINT);
    MPI_Finalize();

    return 0;
}

