#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

// Tolerance for considering something "close to zero"
#define EPSILON 1e-12

// Create an n x n matrix in a single block
double** create_matrix(int n) {
    double* data = (double*) malloc(n * n * sizeof(double));
    double** mat = (double**) malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        mat[i] = &data[i*n];
    }
    return mat;
}

// Free the matrix allocated by create_matrix()
void free_matrix(double** mat) {
    // First free the block of data
    free(mat[0]);
    // Then free the array of pointers
    free(mat);
}

// Read matrix from stdin (still interactive for demonstration)
int read_matrix(double*** A, int* n) {
    printf("Matrix size n: ");
    if (scanf("%d", n) != 1 || *n <= 0) {
        fprintf(stderr, "Invalid matrix size\n");
        return 0;
    }

    // Create matrix
    *A = create_matrix(*n);

    // Fill the matrix
    for (int i = 0; i < *n; i++) {
        printf("Enter row %d of %d values:\n", i+1, *n);
        for (int j = 0; j < *n; j++) {
            if (scanf("%lf", &(*A)[i][j]) != 1) {
                fprintf(stderr, "Invalid input at row %d, col %d\n", i+1, j+1);
                return 0;
            }
        }
    }
    return 1;
}

// Print an n x n matrix
void print_matrix(const char* name, double** mat, int n) {
    printf("\n%s:\n", name);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%10.4f ", mat[i][j]);
        }
        printf("\n");
    }
}

// Perform LU decomposition with partial pivoting
// A is overwritten by the factorization
// L and U are returned in separate matrices for clarity
int lu_decomposition(double** A, int n, double*** L_out, double*** U_out) {
    double** L = create_matrix(n);
    double** U = create_matrix(n);

    // Initialize L as identity, U as copy of A
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            U[i][j] = A[i][j];
            L[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // For each column k, find pivot row p
    for (int k = 0; k < n; k++) {
        // Find row p >= k for which |U[p][k]| is maximal
        double max_val = fabs(U[k][k]);
        int p = k;
        for (int r = k+1; r < n; r++) {
            double val = fabs(U[r][k]);
            if (val > max_val) {
                max_val = val;
                p = r;
            }
        }
        // If pivot is effectively zero, no unique LU decomposition
        if (max_val < EPSILON) {
            fprintf(stderr, "Matrix is singular or near-singular at column %d.\n", k);
            free_matrix(L);
            free_matrix(U);
            return 0;
        }
        // Swap pivot row if needed
        if (p != k) {
            // Swap rows p and k in U
            for (int col = 0; col < n; col++) {
                double tmp = U[k][col];
                U[k][col] = U[p][col];
                U[p][col] = tmp;
            }
            // Swap the corresponding rows in L, but only up to column k
            // because columns >= k haven’t been updated yet
            for (int col = 0; col < k; col++) {
                double tmp = L[k][col];
                L[k][col] = L[p][col];
                L[p][col] = tmp;
            }
        }

        // Eliminate below pivot
        for (int i = k+1; i < n; i++) {
            double factor = U[i][k] / U[k][k];
            L[i][k] = factor;

            for (int j = k; j < n; j++) {
                U[i][j] -= factor * U[k][j];
            }
        }
    }

    *L_out = L;
    *U_out = U;
    return 1;
}

// Multiply two matrices
double** matrix_multiply(double** X, double** Y, int n) {
    double** R = create_matrix(n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += X[i][k] * Y[k][j];
            }
            R[i][j] = sum;
        }
    }
    return R;
}

// Main driver
int main() {
    int n;
    double** A;

    if (!read_matrix(&A, &n)) {
        return 1;
    }

    double** L = NULL;
    double** U = NULL;

    // Perform LU with partial pivoting
    if (!lu_decomposition(A, n, &L, &U)) {
        free_matrix(A);
        return 1;
    }

    // Print L, U (for debugging; skip for large n)
    print_matrix("L", L, n);
    print_matrix("U", U, n);

    // Optional check: L * U = A?
    double** LU = matrix_multiply(L, U, n);

    // Print to see if we indeed get A back
    print_matrix("L*U", LU, n);

    // Cleanup
    free_matrix(A);
    free_matrix(L);
    free_matrix(U);
    free_matrix(LU);

    return 0;
}
