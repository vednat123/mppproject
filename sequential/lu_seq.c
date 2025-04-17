#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

static void generate_random_matrix(double *A, int N)
{
    srand(12345);                        
    for (int i = 0; i < N * N; ++i)
        A[i] = (double)(rand() % 100) + 1.0;
}

static void swap_rows(double *A, int N, int r1, int r2)
{
    if (r1 == r2) return;
    for (int j = 0; j < N; ++j) {
        double tmp = A[r1*N + j];
        A[r1*N + j] = A[r2*N + j];
        A[r2*N + j] = tmp;
    }
}

static int lu_decomposition(double *A, int N)
{
    for (int k = 0; k < N; ++k) {

        // find pivot row 
        double max_val  = fabs(A[k*N + k]);
        int    piv_row  = k;
        for (int i = k + 1; i < N; ++i) {
            double v = fabs(A[i*N + k]);
            if (v > max_val) { max_val = v; piv_row = i; }
        }
        if (piv_row != k) swap_rows(A, N, k, piv_row);

        double pivot = A[k*N + k];
        //singular
        if (fabs(pivot) < 1e-12) return -1;      

        //scale column k
        for (int i = k + 1; i < N; ++i) {
            double factor = A[i*N + k] /= pivot;   //L(i,k) 
            for (int j = k + 1; j < N; ++j)
                A[i*N + j] -= factor * A[k*N + j];
        }
    }
    return 0;
}

static double wall_seconds(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char *argv[])
{
    if (argc < 2) { fprintf(stderr,"usage: %s N\n", argv[0]); return 1; }
    int N = atoi(argv[1]);

    double *A = malloc((size_t)N * N * sizeof *A);
    if (!A) { perror("malloc"); return 1; }
    generate_random_matrix(A, N);

    double t0 = wall_seconds();
    int rc = lu_decomposition(A, N);
    double t1 = wall_seconds();

    if (rc == 0)
        printf("Sequential LU done  |  N=%d  time=%8.4f s\n",
               N, t1 - t0);
    else
        fprintf(stderr,"Matrix is singular or ill‑conditioned (pivot≈0)\n");

    free(A);
    return rc;
}