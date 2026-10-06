/*
 * Tema 2 ASC
 * 2026 Spring
 */
#include "utils.h"
#include <stdlib.h>

double* my_solver(int N, double *A, double *B, double *x) {
    double* C = (double*) calloc(N * N, sizeof(double));
    double* D = (double*) calloc(N * N, sizeof(double));
    double* y = (double*) calloc(N, sizeof(double));

    if (!C || !D || !y) return NULL;

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            for (int k = 0; k < N; k++) {
                C[i * N + j] += A[k * N + i] * B[k * N + j];
            }
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j <= i; j++) {
            double intermediate_sum = 0;
            for (int k = 0; k < N; k++) {
                intermediate_sum += C[i * N + k] * C[j * N + k];
            }
            D[i * N + j] = intermediate_sum;
            if (i != j) {
                D[j * N + i] = intermediate_sum; 
            }
        }
    }

    double* sum_C_rows = (double*) calloc(N, sizeof(double));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            sum_C_rows[j] += C[i * N + j];
        }
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            y[i] += D[i * N + j] * sum_C_rows[j];
        }
    }

    for (int i = 0; i < N; i++) {
        y[i] += x[i];
    }

    free(C);
    free(D);
    free(sum_C_rows);

    return y;
}
