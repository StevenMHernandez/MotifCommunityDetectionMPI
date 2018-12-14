#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <float.h>
#include <math.h>
#include <map>
#include <set>
#include <vector>
#include <mpi.h>
#include "generatePoints.h"
#include "buildDissimilarityMatrix.h"
#include "collectMotifTransitions.h"

int main(int argc, char *argv[]) {
    Config config = Config();

    if (argc > 1) {
        config.POINTS_TO_CREATE = atoi(argv[1]);
    }

    struct timespec start, end, optimizationStart, optimizationEnd;

    int rank = 0, total_tasks = -1;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &total_tasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status stat;

    config.TOTAL_PROCESSORS = total_tasks;
    config.RANK = rank;
    srandom(static_cast<unsigned int>(rank));

    double *points = (double *) malloc(config.POINTS_TO_CREATE * 2 * sizeof(double));

    if (rank == 0) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);
        /// Generate Points in Euclidean Space to use
        generatePoints(points, config.POINTS_TO_CREATE, config);
    }

    MPI_Bcast(points,config.POINTS_TO_CREATE, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    /// For simplicity, we will simply generate L_t1 and L_t2 per process
    /// However, this could easily be run in parallel as well!

    /// Create Dissimilarty matrix at time t=1
    config.DISTANCE_THRESHOLD = 100;
    double **L_t1 = buildDissimilarityMatrix(points, config.POINTS_TO_CREATE, config);

    /// Create Dissimilarty matrix at time t=2
    config.DISTANCE_THRESHOLD = 50;
    double **L_t2 = buildDissimilarityMatrix(points, config.POINTS_TO_CREATE, config);

    /// Create Communities data structure
    std::vector<std::set<int> > C;
    C.resize(static_cast<unsigned long>(config.POINTS_TO_CREATE), std::set<int>());

    clock_gettime(CLOCK_MONOTONIC_RAW, &optimizationStart);
    long *T = collectMotifTransitions(L_t1, L_t2, config);
    clock_gettime(CLOCK_MONOTONIC_RAW, &optimizationEnd);

    if (rank == 0 && config.__PRINT_TIME_TAKEN__) {
        clock_gettime(CLOCK_MONOTONIC_RAW, &end);
        //* Print the time take for the given number of processors used and number of cities total
        //* Format: "identifier,num processors,num points,time(ms)"
//        printf("time_taken_ms,%i,%i,%lf\n", total_tasks, config.POINTS_TO_CREATE, (1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e6);
        printf("optimization_time_taken_ms,%i,%i,%lf\n", total_tasks, config.POINTS_TO_CREATE, (1000000000L * (optimizationEnd.tv_sec - optimizationStart.tv_sec) + optimizationEnd.tv_nsec - optimizationStart.tv_nsec) / 1e6);
    }

    MPI_Finalize();
    return 0;
}
