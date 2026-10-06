/*
 * Tema 2 ASC
 * 2024 Spring
 */
#include "utils.h"
#include <stdlib.h>
#include <cblas.h>
/* 
 * Add your BLAS implementation here
 */
double* my_solver(int N, double *A, double *B, double *x) {
    double* C = (double*) calloc(N * N, sizeof(double));
    double* D = (double*) calloc(N * N, sizeof(double));
    double* y = (double*) calloc(N, sizeof(double));
    double* v = (double*) calloc(N, sizeof(double)); 

    if (!C || !D || !y || !v) {
        free(C); free(D); free(y); free(v);
        return NULL;
    }

    cblas_dgemm(CblasRowMajor, CblasTrans, CblasNoTrans,
                N, N, N, 1.0, A, N, B, N, 0.0, C, N);

    cblas_dsyrk(CblasRowMajor, CblasUpper, CblasNoTrans,
                N, N, 1.0, C, N, 0.0, D, N);

    for (int i = 0; i < N; i++) {
        cblas_daxpy(N, 1.0, &C[i * N], 1, v, 1);
    }

    cblas_dcopy(N, x, 1, y, 1);

    cblas_dsymv(CblasRowMajor, CblasUpper,
                N, 1.0, D, N, v, 1, 1.0, y, 1);

    free(C);
    free(D);
    free(v);

    return y; 
}

