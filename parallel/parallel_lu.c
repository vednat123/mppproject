#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <omp.h>

#define MAX_LINE 65536
#define EPSILON 1e-12

// Matrix creation with parallel allocation
double** create_matrix(int n) {
    double** mat = (double**)malloc(n * sizeof(double*));
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        mat[i] = (double*)malloc(n * sizeof(double));
    }
    return mat;
}

// Matrix deallocation with parallel freeing
void free_matrix(double** mat, int n) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        free(mat[i]);
    }
    free(mat);
}

// Matrix input with validation
int read_matrix(double*** mat, int *n) {
    char buffer[MAX_LINE];
    
    // Get matrix size
    printf("Enter the size of the square matrix (n): ");
    fgets(buffer, MAX_LINE, stdin);
    if (sscanf(buffer, "%d", n) != 1 || *n <= 0) {
        fprintf(stderr, "Invalid matrix size\n");
        return 0;
    }

    // Allocate matrix
    *mat = create_matrix(*n);
    
    // Read matrix rows
    for (int i = 0; i < *n; i++) {
        while (1) {
            printf("Enter row %d with %d space-separated values: ", i+1, *n);
            fgets(buffer, MAX_LINE, stdin);
            
            char* token = strtok(buffer, " \t\n");
            int count = 0;
            while (token != NULL && count < *n) {
                // Check for valid number format
                if (isdigit(token[0]) || token[0] == '-' || 
                    token[0] == '+' || token[0] == '.') {
                    (*mat)[i][count++] = atof(token);
                }
                token = strtok(NULL, " \t\n");
            }
            
            if (count == *n) break;
            fprintf(stderr, "Invalid input. Please enter exactly %d numbers\n", *n);
        }
    }
    return 1;
}

// Parallel matrix printing with critical section
void print_matrix(const char* name, double** mat, int n) {
    printf("\n%s:\n", name);
    #pragma omp parallel for
    for (int i = 0; i < n; i++) {
        #pragma omp critical
        {
            for (int j = 0; j < n; j++) {
                printf("%12.6f ", mat[i][j]);
            }
            printf("\n");
        }
    }
}

// Parallel LU decomposition
int lu_decomposition(double** A, int n, double*** L, double*** U) {
    *L = create_matrix(n);
    *U = create_matrix(n);

    // Initialize matrices in parallel
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            (*U)[i][j] = A[i][j];
            (*L)[i][j] = (i == j) ? 1.0 : 0.0;
        }
    }

    // Main decomposition loop
    for (int k = 0; k < n-1; k++) {
        if (fabs((*U)[k][k]) < EPSILON) {
            fprintf(stderr, "Zero pivot at (%d,%d)\n", k+1, k+1);
            return 0;
        }
        
        // Parallel row operations
        #pragma omp parallel for
        for (int i = k+1; i < n; i++) {
            double multiplier = (*U)[i][k] / (*U)[k][k];
            (*L)[i][k] = multiplier;
            
            for (int j = k; j < n; j++) {
                (*U)[i][j] -= multiplier * (*U)[k][j];
            }
        }
    }
    return 1;
}

// Parallel matrix multiplication
double** matrix_multiply(double** L, double** U, int n) {
    double** result = create_matrix(n);
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            result[i][j] = 0.0;
            for (int k = 0; k < n; k++) {
                result[i][j] += L[i][k] * U[k][j];
            }
        }
    }
    return result;
}

int main() {
    // Set up OpenMP
    omp_set_num_threads(omp_get_max_threads());
    printf("Parallel LU Decomposition Program (Using %d threads)\n", omp_get_max_threads());
    
    int n;
    double** A = NULL;
    double** L = NULL;
    double** U = NULL;

    // Read input matrix
    if (!read_matrix(&A, &n)) {
        return 1;
    }

    // Perform LU decomposition with timing
    double start_time = omp_get_wtime();
    if (!lu_decomposition(A, n, &L, &U)) {
        free_matrix(A, n);
        return 1;
    }
    double decomp_time = omp_get_wtime() - start_time;

    // Print results
    print_matrix("Lower Triangular Matrix (L)", L, n);
    print_matrix("Upper Triangular Matrix (U)", U, n);
    printf("\nDecomposition time: %.6f seconds\n", decomp_time);

    // Verification
    printf("\nVerify decomposition? (y/n): ");
    char c = getchar();
    if (tolower(c) == 'y') {
        start_time = omp_get_wtime();
        double** product = matrix_multiply(L, U, n);
        double mult_time = omp_get_wtime() - start_time;
        
        print_matrix("Product of L and U", product, n);
        printf("Verification time: %.6f seconds\n", mult_time);
        free_matrix(product, n);
    }

    // Clean up memory
    free_matrix(A, n);
    free_matrix(L, n);
    free_matrix(U, n);

    return 0;
}