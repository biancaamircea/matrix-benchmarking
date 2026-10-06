/*
 * Tema 2 ASC
 * 2026 Spring
 */
#include "utils.h"

/*
 * Add your optimized implementation here
 */
#define B_SIZE 40

double* my_solver(int N, double *A, double *B, double *x) {
    double* C = (double*) calloc(N * N, sizeof(double));
    double* y = (double*) calloc(N, sizeof(double));
    double* v = (double*) calloc(N, sizeof(double)); 
    double* w = (double*) calloc(N, sizeof(double)); 

    if (!C || !y || !v || !w) return NULL;

    for (int bi = 0; bi < N; bi += B_SIZE) {
        for (int bk = 0; bk < N; bk += B_SIZE) {
            for (int bj = 0; bj < N; bj += B_SIZE) {
                
                for (int i = bi; i < bi + B_SIZE; i++) {
                    register double *c_row = &C[i * N];
                    
                    for (int k = bk; k < bk + B_SIZE; k++) {
                        register double a_val = A[k * N + i]; 
                        register double *b_row = &B[k * N];
                        
                        for (int j = bj; j < bj + B_SIZE; j += 4) {
                            c_row[j]     += a_val * b_row[j];
                            c_row[j + 1] += a_val * b_row[j + 1];
                            c_row[j + 2] += a_val * b_row[j + 2];
                            c_row[j + 3] += a_val * b_row[j + 3];
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < N; i++) {
        register double *c_row = &C[i * N];
        for (int j = 0; j < N; j++) {
            v[j] += c_row[j];
        }
    }

    for (int j = 0; j < N; j++) {
        register double v_val = v[j];
        register double *c_row = &C[j * N];
        
        for (int i = 0; i < N; i += 4) {
            w[i]     += c_row[i]     * v_val;
            w[i + 1] += c_row[i + 1] * v_val;
            w[i + 2] += c_row[i + 2] * v_val;
            w[i + 3] += c_row[i + 3] * v_val;
        }
    }

    for (int i = 0; i < N; i++) {
        register double *c_row = &C[i * N];
        register double temp_y = 0.0;
        
        for (int j = 0; j < N; j += 4) {
            temp_y += c_row[j]     * w[j]
                    + c_row[j + 1] * w[j + 1]
                    + c_row[j + 2] * w[j + 2]
                    + c_row[j + 3] * w[j + 3];
        }
        y[i] = temp_y + x[i]; 
    }

    free(C);
    free(v);
    free(w);

    return y;
}
