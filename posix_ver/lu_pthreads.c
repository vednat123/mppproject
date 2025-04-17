#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

static int N = 1500;
static int num_threads = 4;
static double *A;

typedef struct {
    pthread_mutex_t m;
    pthread_cond_t  c;
    int             cnt;
    int             tot;
    int             gen;
} barrier_t;

static barrier_t B;
static pthread_mutex_t pivot_m = PTHREAD_MUTEX_INITIALIZER;
static long long tries = 0, locks = 0;

static inline void bar_init(barrier_t *b, int n)
{
    pthread_mutex_init(&b->m, NULL);
    pthread_cond_init(&b->c, NULL);
    b->cnt = b->tot = n;
    b->gen = 0;
}

static inline void bar_wait(barrier_t *b)
{
    pthread_mutex_lock(&b->m);
    int g = b->gen;
    if (--b->cnt == 0) {
        b->gen++;
        b->cnt = b->tot;
        pthread_cond_broadcast(&b->c);
    } else {
        while (g == b->gen)
            pthread_cond_wait(&b->c, &b->m);
    }
    pthread_mutex_unlock(&b->m);
}

static inline void swap(double *a, int n, int r1, int r2)
{
    if (r1 == r2) return;
    for (int j = 0; j < n; ++j) {
        double t = a[r1*n + j];
        a[r1*n + j] = a[r2*n + j];
        a[r2*n + j] = t;
    }
}

static void *worker(void *arg)
{
    int id = (int)(intptr_t)arg;
    for (int k = 0; k < N; ++k) {

        if (!pthread_mutex_trylock(&pivot_m)) {
            locks++;
            double mx = fabs(A[k*N + k]);
            int p = k;
            for (int i = k + 1; i < N; ++i) {
                double v = fabs(A[i*N + k]);
                if (v > mx) { mx = v; p = i; }
            }
            if (p != k) swap(A, N, k, p);
            pthread_mutex_unlock(&pivot_m);
        } else {
            tries++;
            pthread_mutex_lock(&pivot_m);
            pthread_mutex_unlock(&pivot_m);
        }

        bar_wait(&B);

        int rows = N - k - 1;
        int chunk = (rows + num_threads - 1) / num_threads;
        int i0 = k + 1 + id * chunk;
        int i1 = i0 + chunk;
        if (i1 > N) i1 = N;

        for (int i = i0; i < i1; ++i) {
            double f = A[i*N + k] / A[k*N + k];
            A[i*N + k] = f;
            for (int j = k + 1; j < N; ++j)
                A[i*N + j] -= f * A[k*N + j];
        }

        bar_wait(&B);
    }
    return NULL;
}

static void gen(double *a, int n)
{
    srand(12345);
    for (int i = 0; i < n*n; ++i)
        a[i] = (rand() % 100) + 1.0;
}

static double wt(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char **argv)
{
    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) num_threads = atoi(argv[2]);

    A = malloc((size_t)N * N * sizeof *A);
    gen(A, N);

    bar_init(&B, num_threads);

    pthread_t th[num_threads];
    for (int t = 0; t < num_threads; ++t)
        pthread_create(&th[t], NULL, worker, (void *)(intptr_t)t);

    double t0 = wt();
    for (int t = 0; t < num_threads; ++t)
        pthread_join(th[t], NULL);
    double t1 = wt();

    printf("LU Pthreads N=%d T=%d time=%.3f s busy%%=%.2f\n",
           N, num_threads, t1 - t0,
           (double)tries / (tries + locks + 1e-12) * 100.0);

    free(A);
    return 0;
}
