#include <cstdio>
#include <algorithm>
#include "Motif.h"

#define MAX_NUMBER_OF_MOTIFS 8

#define TAG_COLLECT_COUNTS 1

struct Range {
    int min = 0;
    int max = 0;
    int expected = 0; // inclusive range count
};

void calculateSums(long **r, long *T, Range range, Config config) {
    for (int a = range.min; a <= range.max; a++) {
        int a_i = a - range.min;
        long s = 0;
        for (int p = 0; p < config.TOTAL_PROCESSORS; p++) {
            s += r[p][a_i];
        }

        T[a] = s;

        if (config.__PRINT_T_COUNTS__) {
            printf("T_counts,%i,%li\n", a_i, s);
        }
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

long *collectMotifTransitions(double **L_t1, double **L_t2, Config config) {
    /// Initialize matrix
    long *T = (long *) calloc(MAX_NUMBER_OF_MOTIFS * MAX_NUMBER_OF_MOTIFS + 10, sizeof(long *));

    /// Determine ranges of `T` which each processor will be responsible for
    int globalNumRequestsExpected = static_cast<int>(ceil(MAX_NUMBER_OF_MOTIFS * MAX_NUMBER_OF_MOTIFS / config.TOTAL_PROCESSORS));
    int numRequestsExpected = globalNumRequestsExpected;
    Range T_ranges[config.TOTAL_PROCESSORS];
    for (int p = 0; p < config.TOTAL_PROCESSORS; p++) {
        T_ranges[p].min = p * globalNumRequestsExpected;
        if (p == config.TOTAL_PROCESSORS - 1) {
            T_ranges[p].max = (MAX_NUMBER_OF_MOTIFS * MAX_NUMBER_OF_MOTIFS) - 1;
        } else {
            T_ranges[p].max = ((p + 1) * globalNumRequestsExpected) - 1;
        }

        T_ranges[p].expected = (T_ranges[p].max - T_ranges[p].min);

        if (p == config.RANK) {
            numRequestsExpected = T_ranges[p].expected;
        }
    }

    /// Initialize data expected to be collected
    long **r;
    r = (long **) malloc(config.TOTAL_PROCESSORS * sizeof(long *));
    for (int i = 0; i < config.TOTAL_PROCESSORS; i++) {
        r[i] = (long *) calloc(static_cast<size_t>(numRequestsExpected), sizeof(long));
    }

    /// Initialize Asynchronous MPI Receiving
    MPI_Request mRequests[config.TOTAL_PROCESSORS];
    MPI_Status mStatuses[config.TOTAL_PROCESSORS];
    for (int p = 0; p < config.TOTAL_PROCESSORS; p++) {
        MPI_Irecv(r[p], numRequestsExpected, MPI_LONG, p, TAG_COLLECT_COUNTS, MPI_COMM_WORLD, &mRequests[p]);
    }

    /// for all v \in G
    Range ran = Range();
    int ps = static_cast<int>(ceil(config.POINTS_TO_CREATE / config.TOTAL_PROCESSORS));
    ran.min = ps * config.RANK;
    ran.max = (ps * (config.RANK + 1)) - 1;
    if (config.RANK == config.TOTAL_PROCESSORS - 1) {
        ran.max = std::min(ran.max, config.POINTS_TO_CREATE);
    }

    for (int v = ran.min; v < ran.max; v++) {
        Motif *m = new Motif(L_t1);
        m->add(v);
        std::vector<Motif*> M = getSubGraph(config.K - 1, L_t1, m, config);

        /// Count all motif transitions locally
        for (int m = 0; m < M.size(); m++) {
            Motif *m_curr = M[m];
            Motif *m_next = getNextInstanceAsMotif(L_t1, L_t2, m_curr);
            int index = (m_curr->identifier * MAX_NUMBER_OF_MOTIFS) + m_next->identifier;
            T[index] = T[index] + 1;
        }
    }

    /// Send local sums out for global summation
    for (int p = 0; p < config.TOTAL_PROCESSORS; p++) {
        int count = T_ranges[p].expected;
        MPI_Send(&T[T_ranges[config.RANK].min], count, MPI_LONG, p, TAG_COLLECT_COUNTS, MPI_COMM_WORLD);
    }

    MPI_Waitall(config.TOTAL_PROCESSORS, mRequests, mStatuses);
    MPI_Barrier(MPI_COMM_WORLD);

    calculateSums(r, T, T_ranges[config.RANK], config);

    return T;
}