#ifndef MOTIFCOMMUNITYDETECTIONMPI_CONFIG_H
#define MOTIFCOMMUNITYDETECTIONMPI_CONFIG_H

class Config {
public:
    int    K = 3;
    double DISTANCE_THRESHOLD = 10;
    int    POINTS_TO_CREATE = 100;
    int    RAND_POINT_MAX = 100;
    int    TOTAL_PROCESSORS = -1;
    int    RANK = -1;

    bool __PRINT_RANDOM_POINTS__ = false;
    bool __PRINT_EDGES__         = false;
    bool __PRINT_EDGE_COUNT__    = true;
    bool __PRINT_TIME_TAKEN__    = true;
    bool __PRINT_T_COUNTS__      = false;
};

#endif //MOTIFCOMMUNITYDETECTIONMPI_CONFIG_H
