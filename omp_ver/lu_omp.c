#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

static void generate_random_matrix(double *A, int N)
{
    srand(12345);
    for (int i = 0; i < N * N; ++i)
        A[i] = (double)(rand() % 100) + 1.0;
}

static inline void swap_rows(double *A, int N, int r1, int r2)
{
    if (r1 == r2) return;
    for (int j = 0; j < N; ++j) {
        double tmp = A[r1 * N + j];
        A[r1 * N + j] = A[r2 * N + j];
        A[r2 * N + j] = tmp;
    }
}

static void lu_decomposition_omp(double *A, int N)
{
#pragma omp parallel
    {
        for (int k = 0; k < N; ++k) {

            #pragma omp single
            {
                double max_val = fabs(A[k * N + k]);
                int    piv_row = k;

                for (int i = k + 1; i < N; ++i) {
                    double v = fabs(A[i * N + k]);
                    if (v > max_val) {
                        max_val = v;
                        piv_row = i;
                    }
                }

                if (piv_row != k)
                    swap_rows(A, N, k, piv_row);

                if (fabs(A[k * N + k]) < 1e-12)
                    fprintf(stderr, "Warning: near zero pivot at k=%d\n", k);
            }

            #pragma omp for schedule(static)
            for (int i = k + 1; i < N; ++i) {
                A[i * N + k] /= A[k * N + k];
            }

            #pragma omp for schedule(static)
            for (int i = k + 1; i < N; ++i) {
                double factor = A[i * N + k];

                #pragma omp simd
                for (int j = k + 1; j < N; ++j) {
                    A[i * N + j] -= factor * A[k * N + j];
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s N [threads]\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    if (argc >= 3)
        omp_set_num_threads(atoi(argv[2]));

    double *A = malloc((size_t)N * N * sizeof *A);
    if (!A) {
        perror("malloc");
        return 1;
    }

    generate_random_matrix(A, N);

    double t0 = omp_get_wtime();
    lu_decomposition_omp(A, N);
    double t1 = omp_get_wtime();

    printf("OpenMP LU done  |  N=%d  threads=%d  time=%8.4f s\n",
           N, omp_get_max_threads(), t1 - t0);

    free(A);
    return 0;
}