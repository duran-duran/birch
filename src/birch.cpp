#include <chrono>
#include "mpi.h"

#include "common.h"
#include "metrics.h"
#include "cftree.h"
#include "cftreebuilder.h"

int rank = 0,
    procs = 1;

#define MPI_DATA_T MPI_DOUBLE
MPI_Datatype DATA_POINT;

DataPoint generateDataPoint(size_t dim, data_t min, data_t max);
CF_Vector generateAndDistributeData(int count, int dim, data_t min, data_t max);
std::vector<CF_Vector> distrKMeans(const CF_Vector &entries, size_t dim);

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &procs);

    if (rank == ROOT) LOG_DEBUG("Total number of processes: %d", procs);

    srand(time(NULL));

    int dim;
    data_t min, max;
    int start, finish;
    int expCnt;

    if (rank == ROOT) {
        if (argc < 7) {
            LOG("Usage:");
            LOG("\tbirch <dimensions> <min> <max> <points_start> <points_finish> <experiments_count>");
            return 1;
        }

        dim = atoi(argv[1]);
        min = atof(argv[2]);
        max = atof(argv[3]);
        start = atoi(argv[4]);
        finish = atoi(argv[5]);
        expCnt = atoi(argv[6]);

        LOG_DEBUG("Input parameters:");
        LOG_DEBUG("%d-dimensional data points from [%f, %f]", dim, min, max);
        LOG_DEBUG("%d experiments from %d points to %ld points", expCnt, start, finish);
    }

    MPI_Bcast(&dim, 1, MPI_INT, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&min, 1, MPI_DATA_T, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&max, 1, MPI_DATA_T, ROOT, MPI_COMM_WORLD);
    MPI_Bcast(&expCnt, 1, MPI_INT, ROOT, MPI_COMM_WORLD);

    MPI_Type_contiguous(dim, MPI_DATA_T, &DATA_POINT);
    MPI_Type_commit(&DATA_POINT);

    std::chrono::time_point<std::chrono::system_clock> begin, end;

    for (int i = 0; i < expCnt; ++i)
    {
        LOG_DEBUG("Starting %d experiment..", i + 1);

        if (rank == ROOT) begin = std::chrono::system_clock::now();

        int count = (expCnt > 1) ? (start + (finish - start) * i / (expCnt - 1)) : start;
        auto entries = generateAndDistributeData(count, dim, min, max);
        LOG_DEBUG("Total number of acquired clusters: %d", entries.size());
        auto clusters = distrKMeans(entries, dim);

        if (rank == ROOT)
        {
            end = std::chrono::system_clock::now();
            std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
            LOG_DEBUG("Total elapsed time: %ldms", elapsed.count());

            LOG("%d %d %d", procs, count, elapsed);
        }
    }

    MPI_Finalize();

    return 0;
}

CF_Vector generateAndDistributeData(int count, int dim, data_t min, data_t max) {
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
//            LOG_DEBUG("Reading %d data point...", i + 1);
            buff[r] = generateDataPoint(dim, min, max);
            if (r == 0) {
                DataPoint dataPoint(&buff[r][0], dim);
                treeBuilder.addPointToTree(dataPoint);
            } else if (r < procs) {
//                LOG_DEBUG("Sending data point to process %d", r);
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

std::vector<CF_Vector> distrKMeans(const CF_Vector &entries, size_t dim)
{
    int localEntries = entries.size(),
        globalEntries = 0;
    MPI_Allreduce(&localEntries, &globalEntries, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
    LOG_DEBUG("Total number of cfs acquired among all processes: %d", globalEntries);

    int k = std::sqrt(globalEntries);
    LOG_DEBUG("Starting k-means algorithm with k = %d", k);

    std::vector<DataPoint> centroidSeeds(k, DataPoint((data_t)0, dim));
    if (rank == ROOT)
    {
        auto allData = CF_Cluster(entries);
        for (int j = 0; j < k; ++j)
        {
            centroidSeeds[j] = allData.X0;
            for (size_t i = 0; i < dim; ++i)
                centroidSeeds[j][i] += (double)rand() / RAND_MAX * allData.R;
        }
    }

    for (int j = 0; j < k; ++j)
    {
        MPI_Bcast(&centroidSeeds[j][0], 1, DATA_POINT, ROOT, MPI_COMM_WORLD);
        LOG_DEBUG("%d centroid: %s", j + 1, pointToString(centroidSeeds[j]).c_str());
    }

    CF_Vector centroids;
    for (int j = 0; j < k; ++j)
        centroids.emplace_back(centroidSeeds[j]);
    centroidSeeds.clear();

    std::vector<CF_Vector> clusters(k);
    data_t MSE = std::numeric_limits<data_t>::max(),
           newMSE = MSE,
           localMSE;
    std::vector<int> localN(k), globalN(k);
    std::vector<DataPoint> localSum(k, DataPoint((data_t)0, dim)), globalSum(k, DataPoint((data_t)0, dim));
    do
    {
        MSE = newMSE;
        localMSE = 0;
        for (int j = 0; j < k; ++j)
        {
            localN[j] = 0;
            localSum[j] = DataPoint((data_t)0, dim);
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
        for (int j = 0; j < k; ++j)
        {
            MPI_Allreduce(&localN[j], &globalN[j], 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
//            LOG_DEBUG("Total number of points in %d cluster: %d", j + 1, globalN[j]);
            MPI_Allreduce(&localSum[j][0], &globalSum[j][0], dim, MPI_DATA_T, MPI_SUM, MPI_COMM_WORLD);
            if (globalN[j] != 0)
                centroids[j] = CF_Cluster(globalSum[j] / (data_t) globalN[j]);
//            LOG_DEBUG("New %d cluster: %s", j + 1, pointToString(centroids[j].X0).c_str());
        }
        MPI_Allreduce(&localMSE, &newMSE, 1, MPI_DATA_T, MPI_SUM, MPI_COMM_WORLD);
        LOG_DEBUG("New MSE: %f", newMSE);
    }
    while (newMSE < MSE);

    return clusters;
}

DataPoint generateDataPoint(size_t dim, data_t min, data_t max)
{
    DataPoint point((data_t) 0, dim);
    for (size_t i = 0; i < dim; ++i)
        point[i] = ((data_t) rand() / RAND_MAX) * (max - min) + min;
    return point;
}
