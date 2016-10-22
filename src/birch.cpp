#include "common.h"
#include "mpi.h"
#include <vector>
#include <iostream>

#define MPI_DATA_T MPI_DOUBLE

void gotDataPoint(int rank, const DataPoint& point)
{
    std::cout << "Process " << rank << ": got data point:";
    for (size_t i = 0; i < point.size(); ++i) {
        std::cout << " " << point[i];
    }
    std::cout << std::endl;
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int rank, procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    long count;
    int dim;
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

    std::vector<int> chunks(procs);
    if (rank == 0)
    {
        for (int r = 0; r < procs; ++r)
        {
            chunks[r] = (r < count % procs) ? count / procs + 1 : count / procs;
            std::cout << "Chunk " << r << ": " << chunks[r] << std::endl;
        }
    }

    int chunk;
    MPI_Scatter(&chunks[0], 1, MPI_INT,
                &chunk, 1, MPI_INT,
                0, MPI_COMM_WORLD);
    chunks.clear();

    std::cout << "Process " << rank << ": I'll take " << chunk << " data points" << std::endl;
    if (rank == 0)
    {
        std::vector<DataPoint> buff(procs, DataPoint(dim));
        std::vector<MPI_Request> requests(procs - 1);
        size_t reqCount = 0;
        for (int i = 0, r = 0; i < count; ++i, ++r)
        {
            if (r == procs)
            {
                MPI_Waitall(reqCount, &requests[0], MPI_STATUSES_IGNORE);
                r = reqCount = 0;
            };
            std::cout << "Reading " << i + 1 << "th point.." << std::endl;
            fread(&buff[r][0], point_size, 1, pfile);
            if (r == 0)
            {
                DataPoint dataPoint(&buff[r][0], dim);
                gotDataPoint(rank, dataPoint);
            }
            else if (r < procs)
            {
                std::cout << "Process 0: sending data point to process " << r << std::endl;
                MPI_Isend(&buff[r][0], 1, DATA_POINT, r, 0, MPI_COMM_WORLD, &requests[r - 1]);
                ++reqCount;
            }
        }

        if (reqCount > 0)
        {
            MPI_Waitall(reqCount, &requests[0], MPI_STATUSES_IGNORE);
        }
        fclose(pfile);
        buff.clear();
        requests.clear();
    }
    else
    {
        DataPoint dataPoint(dim);
        for (int i = 0; i < chunk; ++i)
        {
            MPI_Recv(&dataPoint[0], 1, DATA_POINT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            gotDataPoint(rank, dataPoint);
        }
    }

    MPI_Type_free(&DATA_POINT);
    MPI_Finalize();

    return 0;
}

