#include <cstdio>
#include <algorithm>
#include "Motif.h"

#define MAX_NUMBER_OF_MOTIFS 13

#define TAG_COLLECT_COUNTS 1

struct Range {
    int min = 0;
    int max = 0;
};

long **r;

void calculateSums(int **T, Range range, Config config) {
    for (int a = range.min; a <= range.max; a++) {
        int a_i = a - range.min;
        long s = 0;
        for (int p = 0; p < config.TOTAL_PROCESSORS; p++) {
            s += r[a_i][p];
        }

        if (config.__PRINT_T_COUNTS__) {
            printf("T_counts,%i,%i,%li\n", config.RANK, a, s);
        }

//        T[]
    }

    // TODO: broadcast T_{i,j}
}

std::vector<Motif *> getSubGraph(int k, double **L_t1, Motif *M, Config config) {
    if (k <= 0) {
        if (M->isLowest) { // Only return those motifs where the first data point has the lowest index
            std::vector<Motif *> result = std::vector<Motif *>();
            result.push_back(M);
            return result;
        } else {
            return std::vector<Motif *>();
        }
    }
    std::vector<Motif *> M_all = std::vector<Motif *>();

    // for all neighbors, n
    for (int m = 0; m < M->count; m++) {
        for (int n = 0; n < config.POINTS_TO_CREATE; n++) {
            if (L_t1[m][n] > 0 && !M->contains(n)) {
                Motif *M_copy2 = M->copy();
                M_copy2->add(n);
                std::vector<Motif *> M_all_sub = getSubGraph(k - 1, L_t1, M_copy2, config);
                M_all.insert(M_all.end(), M_all_sub.begin(), M_all_sub.end());
            }
        }
    }

    return M_all;
}

Motif *getNextInstanceAsMotif(double **L_t1, double **L_t2, Motif *m_curr) {
    Motif *m_new = new Motif(L_t2);

    for (int m = 0; m < m_curr->count; m++) {
        m_new->add(m_curr->data[m]);
    }

    return m_new;
}

int **collectMotifTransitions(double **L_t1, double **L_t2, Config config) {
    /// Initialize matrix
    int **T = (int **) malloc(MAX_NUMBER_OF_MOTIFS * sizeof(int *));
    for (int i = 0; i < MAX_NUMBER_OF_MOTIFS; i++) {
        T[i] = (int *) calloc(MAX_NUMBER_OF_MOTIFS, sizeof(int *));
    }

    /// Determine ranges of `T` which each processor will be responsible for
    int globalNumRequestsExpected = static_cast<int>(ceil(MAX_NUMBER_OF_MOTIFS * MAX_NUMBER_OF_MOTIFS / config.TOTAL_PROCESSORS));
    int numRequestsExpected = globalNumRequestsExpected;
    Range T_ranges[config.TOTAL_PROCESSORS];
    for (int p = 0; p < config.TOTAL_PROCESSORS; p++) {
        T_ranges[p].min = p * globalNumRequestsExpected;
        if (p == config.TOTAL_PROCESSORS - 1) {
            T_ranges[p].max = (MAX_NUMBER_OF_MOTIFS * MAX_NUMBER_OF_MOTIFS) - 1;
            numRequestsExpected = T_ranges[p].max - T_ranges[p].min;
        } else {
            T_ranges[p].max = ((p + 1) * globalNumRequestsExpected) - 1;
        }
        int range = T_ranges[p].max - T_ranges[p].min;
//        printf("p=%i good golly %i, but %i and r=%i\n", p, T_ranges[p].max, globalNumRequestsExpected, range);
    }


    /// Initialize data expected to be collected
    r = (long **) malloc(numRequestsExpected * sizeof(long *));
    for (int i = 0; i <= numRequestsExpected; i++) {
        r[i] = (long *) calloc(static_cast<size_t>(config.TOTAL_PROCESSORS), sizeof(long));
    }

    /// Initialize Asynchronous MPI Receiving
    MPI_Request mRequests[numRequestsExpected];
    MPI_Status mStatuses[numRequestsExpected];
    for (int a = T_ranges[config.RANK].min; a < T_ranges[config.RANK].max; a++) {
        for (int p = 0; p < config.TOTAL_PROCESSORS; p++) {
            int ij = a - T_ranges[config.RANK].min;
            MPI_Irecv(&r[ij][p], a, MPI_INT, p, TAG_COLLECT_COUNTS, MPI_COMM_WORLD, &mRequests[ij]);
        }
    }

    /// for all v \in G
    Range ran = Range();
    int ps = ceil(config.POINTS_TO_CREATE / config.TOTAL_PROCESSORS);
    ran.min = ps * config.RANK;
    ran.max = (ps * (config.RANK + 1)) - 1;
    if (config.RANK == config.TOTAL_PROCESSORS - 1) {
        ran.max = std::min(ran.max, config.POINTS_TO_CREATE);
    }

    for (int v = ran.min; v < ran.max; v++) {
        Motif *m = new Motif(L_t1);
        m->add(v);
        std::vector<Motif*> M = getSubGraph(config.K - 1, L_t1, m, config);

        for (auto m_curr : M) {
            Motif *m_next = getNextInstanceAsMotif(L_t1, L_t2, m_curr);
            T[m_curr->identifier][m_next->identifier] = T[m_curr->identifier][m_next->identifier] + 1;
        }

        for (int i = 0; i < MAX_NUMBER_OF_MOTIFS; i++) {
            for (int j = 0; j < MAX_NUMBER_OF_MOTIFS; j++) {
                int tag = (MAX_NUMBER_OF_MOTIFS * i) + j;
                int dest = 0;
                for (int k = 0; k < config.TOTAL_PROCESSORS; k++) {
                    if (tag <= T_ranges[k].max && tag >= T_ranges[k].min) {
                        dest = k;
                    }
                }

                //MPI_Send(&T[i][j], 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
            }
        }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    calculateSums(T, T_ranges[config.RANK], config);

    return T;
}