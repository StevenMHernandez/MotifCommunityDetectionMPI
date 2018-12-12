double **buildDissimilarityMatrix(double *points, int numPoints, Config config) {
    int edgeCount = 0;

    /// Create Dissimilarity Matrix := in the future, this would be an
    double **L = (double **) malloc(numPoints * sizeof(double *));
    for (int i = 0; i < numPoints; i++) {
        L[i] = (double *) malloc(numPoints * sizeof(double));

        for (int j = 0; j < numPoints; j++) {
            double d = distance(points[i * 2], points[(i * 2) + 1], points[j * 2], points[(j * 2) + 1]);
            L[i][j] = d < config.DISTANCE_THRESHOLD ? d : -1;
//                L[i][j] = d < DISTANCE_THRESHOLD ? 1 : -1;

            if (L[i][j] != -1) {
                edgeCount++;

                if (config.__PRINT_EDGES__) {
                    //* Print the adjacency matrix values (and the associated weights)
                    //* Format: "identifier,i,j,w(i,j)"
                    printf("adjacency_matrix,%i,%i,%lf\n", i, j, L[i][j]);
                }
            }
        }
    }

    if (config.__PRINT_EDGE_COUNT__) {
        //* Print edge count
        //* Format: "identifier,edge_count"
        printf("edge_count,%i\n", edgeCount);
    }

    return L;
}