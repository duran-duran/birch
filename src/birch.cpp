#include "mpi.h"
#include <iostream>
#include <sstream>

#include "common.h"
#include "metrics.h"
#include "cftree.h"
#include "cftreebuilder.h"

int rank = 0,
    procs = 1;

#define MPI_DATA_T MPI_DOUBLE
MPI_Datatype DATA_POINT;

void gotDataPoint(const DataPoint& point) {
    std::stringstream pointSS;
    for (size_t i = 0; i < point.size(); ++i) {
        pointSS << " " << point[i];
    }
    LOG("got data point: %s", pointSS.str().c_str());
}

void readInputParameters(FILE *pfile, long &count, int &dim);
CF_Vector readAndDistributeData(FILE *pfile, long count, int dim);
std::vector<CF_Vector> distrKMeans(const CF_Vector &entries, int dim);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    srand(time(NULL));

    long count;
    int dim;
    FILE *pfile;

    if (rank == ROOT) {
        if (argc < 2) {
            std::cout << "Usage:" << std::endl;
            std::cout << "\tbirch <input_file> <output_file>" << std::endl;
            return 1;
        }
        char *filename = argv[1];
        pfile = fopen(filename, "rb");
        readInputParameters(pfile, count, dim);
    }

    MPI_Bcast(&dim, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Type_contiguous(dim, MPI_DATA_T, &DATA_POINT);
    MPI_Type_commit(&DATA_POINT);

    auto entries = readAndDistributeData(pfile, count, dim);
    if (rank == ROOT) fclose(pfile);

    auto clusters = distrKMeans(entries, dim);

    MPI_Type_free(&DATA_POINT);
    MPI_Finalize();

    return 0;
}

void readInputParameters(FILE *pfile, long &count, int &dim) {
    fread(&count, sizeof(count), 1, pfile);
    fread(&dim, sizeof(dim), 1, pfile);

    LOG("Input data: %ld %d-dimensional data points", count, dim);
}

CF_Vector readAndDistributeData(FILE *pfile, long count, int dim) {
    const int point_size = sizeof(data_t) * dim;

    std::vector<int> chunks(procs);
    if (rank == ROOT) {
        for (int r = 0; r < procs; ++r) {
            chunks[r] = (r < count % procs) ? count / procs + 1 : count / procs;
        }
    }

    int chunk;
    MPI_Scatter(&chunks[0], 1, MPI_INT,
                &chunk, 1, MPI_INT,
                ROOT, MPI_COMM_WORLD);
    chunks.clear();
    LOG("Expecting %d data_points", chunk);

    CF_TreeBuilder treeBuilder(chunk, dim, std::log(chunk), 0, std::sqrt(chunk), std::log(chunk));

    if (rank == ROOT) {
        std::vector<DataPoint> buff(procs, DataPoint(dim));
        std::vector<MPI_Request> requests(procs - 1);
        size_t reqCount = 0;
        for (int i = 0, r = 0; i < count; ++i, ++r) {
            if (r == procs) {
                MPI_Waitall(reqCount, &requests[0], MPI_STATUSES_IGNORE);
                r = reqCount = 0;
            };
            LOG("Reading %d data point...", i + 1);
            fread(&buff[r][0], point_size, 1, pfile);
            if (r == 0) {
                DataPoint dataPoint(&buff[r][0], dim);
                treeBuilder.addPointToTree(dataPoint);
            } else if (r < procs) {
                LOG("Sending data point to process %d", r);
                MPI_Isend(&buff[r][0], 1, DATA_POINT, r, 0, MPI_COMM_WORLD, &requests[r - 1]);
                ++reqCount;
            }
        }
        MPI_Waitall(reqCount, &requests[0], MPI_STATUSES_IGNORE);

        buff.clear();
        requests.clear();
    } else {
        DataPoint dataPoint(dim);
        for (int i = 0; i < chunk; ++i) {
            MPI_Recv(&dataPoint[0], 1, DATA_POINT, ROOT, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            treeBuilder.addPointToTree(dataPoint);
        }
    }

    return treeBuilder.getAllLeafEntries();
}

std::vector<CF_Vector> distrKMeans(const CF_Vector &entries, int dim)
{
    int k;
    if (rank == ROOT)
        k = std::sqrt(entries.size() * procs);
    MPI_Bcast(&k, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    std::vector<DataPoint> centroidSeeds(k, DataPoint(dim, 0));
    if (rank == ROOT)
    {
        auto allData = CF_Cluster(entries);
        for (int j = 0; j < k; ++j)
            centroidSeeds[j] = DataPoint{(double)rand() / RAND_MAX * allData.R, (double)rand() / RAND_MAX * allData.R} + allData.X0;
    }

    for (int j = 0; j < k; ++j)
        MPI_Bcast(&centroidSeeds[j], 1, DATA_POINT, ROOT, MPI_COMM_WORLD);

    CF_Vector centroids;
    for (int j = 0; j < k; ++j)
        centroids.emplace_back(centroidSeeds[j]);
    centroidSeeds.clear();

    std::vector<CF_Vector> clusters(k);
    data_t MSE = std::numeric_limits<data_t>::max(),
           newMSE = MSE,
           localMSE;
    std::vector<int> localN(k), globalN(k);
    std::vector<DataPoint> localSum(k, DataPoint(dim, 0)), globalSum(k, DataPoint(dim, 0));
    do
    {
        MSE = newMSE;
        localMSE = 0;
        for (int j = 0; j < k; ++j)
        {
            localN[j] = 0;
            localSum[j] = DataPoint(dim, 0);
            clusters[j].clear();
        }
        for (size_t i = 0; i < entries.size(); ++i)
        {
            auto closest = entries[i].findClosest(centroids);
            size_t ind = closest - centroids.begin();

            clusters[ind].push_back(entries[i]);
            localN[ind] += entries[i].N;
            localSum[ind] += entries[i].LS;

            data_t dist = getDistance(entries[i], *closest);
            localMSE += dist*dist;
        }
        for (size_t j = 0; j < centroids.size(); ++j)
        {
            MPI_Allreduce(&localN[j], &globalN[j], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
            MPI_Allreduce(&localSum[j][0], &globalSum[j][0], dim, MPI_DATA_T, MPI_SUM, MPI_COMM_WORLD);
            if (!clusters[j].empty())
                centroids[j] = CF_Cluster(globalSum[j] / (data_t) globalN[j]);
        }
        MPI_Allreduce(&localMSE, &newMSE, 1, MPI_DATA_T, MPI_SUM, MPI_COMM_WORLD);
    }
    while (newMSE < MSE);

    return clusters;
}
