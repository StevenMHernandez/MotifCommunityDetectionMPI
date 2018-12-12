#ifndef MOTIFCOMMUNITYDETECTIONMPI_CONFIG_H
#define MOTIFCOMMUNITYDETECTIONMPI_CONFIG_H

class Config {
public:
    double DISTANCE_THRESHOLD = 10;
    int    POINTS_TO_CREATE = 100;
    int    RAND_POINT_MAX = 100;

    bool __PRINT_RANDOM_POINTS__ = false;
    bool __PRINT_EDGES__         = false;
    bool __PRINT_EDGE_COUNT__    = false;
    bool __PRINT_TIME_TAKEN__    = true;
};

#endif //MOTIFCOMMUNITYDETECTIONMPI_CONFIG_H
