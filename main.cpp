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

int main(int argc, char *argv[]) {
    Config config;

    int rank = 0, total_tasks = -1;
//    MPI_Init(&argc, &argv);
//    MPI_Comm_size(MPI_COMM_WORLD, &total_tasks);
//    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
//    MPI_Status stat;
    if (rank == 0) { // TODO: remove!
        srandom(rank);

        double *points = (double *) malloc(config.POINTS_TO_CREATE * 2 * sizeof(double));

        struct timespec start, end, optimizationStart, optimizationEnd;

        /// Generate Points in Euclidean Space to use
        generatePoints(points, config.POINTS_TO_CREATE, config);
        clock_gettime(CLOCK_MONOTONIC_RAW, &start);

        /// Create Communities data structure
        std::vector<std::set<int> > C;
        C.resize(static_cast<unsigned long>(config.POINTS_TO_CREATE), std::set<int>());

        clock_gettime(CLOCK_MONOTONIC_RAW, &optimizationStart);
        printf("TODO: run optimization algorithm\n");
        clock_gettime(CLOCK_MONOTONIC_RAW, &optimizationEnd);

        if (config.__PRINT_TIME_TAKEN__) {
            clock_gettime(CLOCK_MONOTONIC_RAW, &end);
            //* Print the time take for the given number of processors used and number of cities total
            //* Format: "identifier,num processors,num cities,time(ms)"
            printf("time_taken_ms,%i,%lf\n", total_tasks, (1000000000L * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec) / 1e6);
            printf("optimization_time_taken_ms,%i,%lf\n", total_tasks, (1000000000L * (optimizationEnd.tv_sec - optimizationStart.tv_sec) + optimizationEnd.tv_nsec - optimizationStart.tv_nsec) / 1e6);
        }
    } // TODO: endRemove!

//    MPI_Finalize();
    return 0;
}
