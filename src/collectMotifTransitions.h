#include "Motif.h"

#define MAX_NUMBER_OF_MOTIFS 13

// TODO: remove this hard coded value, consider handling this differently (with an actual struct!)

int *r = (int *) calloc(Config().TOTAL_PROCESSORS, sizeof(int));
int r_total = 0;

void onReceive(int senderId, int T_ij, int i, int j, Config config) {
    r[senderId] = T_ij;
    r_total++;

    if (r_total >= config.TOTAL_PROCESSORS) {
        int s = 0;
        for (int i = 0; i < config.TOTAL_PROCESSORS; i++) {
            s += r[i];
        }

        // TODO: broadcast T_{i,j}
        if (config.__PRINT_T_COUNTS__) {
            //* Print counts of transitions from i to j
            //* Format: "identifier,i,j,num_instances"
            printf("T_counts,%i,%i,%i\n", i, j, T_ij);
        }
    }
}

std::vector<Motif*> getSubGraph(int k, double **L_t1, Motif *M, Config config) {
    if (k <= 0) {
        if (M->isLowest) { // Only return those motifs where the first data point has the lowest index
            std::vector<Motif*> result = std::vector<Motif*>();
            result.push_back(M);
            return result;
        } else {
            return std::vector<Motif*>();
        }
    }
    std::vector<Motif*> M_all = std::vector<Motif*>();

    // for all neighbors, n
    for (int m = 0; m < M->count; m++) {
        for (int n = 0; n < config.POINTS_TO_CREATE; n++) {
            if (L_t1[m][n] > 0 && !M->contains(n)) {
                Motif *M_copy2 = M->copy();
                M_copy2->add(n);
                std::vector<Motif*> M_all_sub = getSubGraph(k - 1, L_t1, M_copy2, config);
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

    /// for all v \in G
    for (int v = 0; v < config.POINTS_TO_CREATE; v++) {
        Motif *m = new Motif(L_t1);
        m->add(v);
        std::vector<Motif*> M = getSubGraph(config.K - 1, L_t1, m, config);

        for (int m = 0; m < M.size(); m++) {
            Motif *m_curr = M[m];
            Motif *m_next = getNextInstanceAsMotif(L_t1, L_t2, m_curr);
            T[m_curr->identifier][m_next->identifier] = T[m_curr->identifier][m_next->identifier] + 1;
        }

        for (int i = 0; i < MAX_NUMBER_OF_MOTIFS; i++) {
            for (int j = 0; j < MAX_NUMBER_OF_MOTIFS; j++) {
                // TODO: send to specific processors!
                onReceive(0, T[i][j], i, j, config);// TODO: remove
            }
        }
    }

    return T;
}