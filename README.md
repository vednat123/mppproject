# LU Decomposition: Sequential, OpenMP & Pthreads

An implementation and performance comparison of LU decomposition using three parallel programming approaches in C.

🔗 **[Live Interactive Demo](https://vednat123.github.io/mppproject/)** — Visualize the algorithm step-by-step

## Overview

LU decomposition factors a square matrix **A** into the product of:
- **L** — Lower triangular matrix
- **U** — Upper triangular matrix

Such that **A = L × U**

This is fundamental for solving systems of linear equations, computing determinants, and matrix inversion. The algorithm's O(N³) complexity makes it an excellent candidate for parallelization.

## Implementations

| Version | File | Description |
|---------|------|-------------|
| Sequential | `sequential.c` | Baseline Gaussian elimination with partial pivoting |
| OpenMP | `openmp.c` | Parallel version using OpenMP directives |
| Pthreads | `pthreads.c` | POSIX threads with custom barrier synchronization |

### Sequential
Standard Gaussian elimination with partial pivoting. The inner loop updates one row at a time.

### OpenMP
Uses `#pragma omp single` for pivot selection and row swapping, with two `omp for` loops to parallelize:
1. The scale step (computing L values)
2. The elimination step (updating U values)

### Pthreads
Implements a custom barrier for synchronization. A global mutex protects the pivot search, and each thread processes its assigned slice of rows.

## Performance Results

**Test Environment:**
- CPU: 8-core / 16-thread AMD Ryzen 9 5900HS
- RAM: 16 GB
- OS: Ubuntu 24.04
- Compiler: GCC 14.1 with `-O3 -march=native`

### OpenMP Results

| Matrix Size | Sequential | 1T | 2T | 4T | 8T | 16T |
|-------------|------------|------|------|------|------|------|
| 500×500 | 0.0255s | 0.0141s | 0.0082s | 0.0061s | 0.0066s | 0.0307s |
| 1000×1000 | 0.2021s | 0.1065s | 0.0558s | 0.0471s | 0.0327s | 0.0612s |
| 1500×1500 | 1.1206s | 0.7358s | 0.3470s | 0.2036s | 0.2868s | 0.3013s |
| 2000×2000 | 2.9234s | 2.7630s | 2.0936s | 1.8705s | 1.6748s | 2.5432s |

**Peak Speedup: ~6× with 8 threads**

### Pthreads Results

| Matrix Size | Sequential | 1T | 2T | 4T | 8T | 16T |
|-------------|------------|------|------|------|------|------|
| 500×500 | 0.0255s | 0.0118s | 0.0278s | 0.0496s | 0.0875s | 0.1669s |
| 1000×1000 | 0.2021s | 0.0990s | 0.0920s | 0.1188s | 0.2212s | 0.3476s |
| 1500×1500 | 1.1206s | 0.7468s | 0.4443s | 0.3638s | 0.5221s | 0.7597s |
| 2000×2000 | 2.9234s | 2.5724s | 1.4303s | 1.4503s | 1.6226s | 1.9806s |

**Peak Speedup: ~3× with 2-4 threads**

## Key Findings

- **OpenMP scales well** up to 8 threads, with diminishing returns beyond that due to pivot step overhead and memory bandwidth limitations
- **Pthreads shows smaller speedups** — the custom barrier and pivot mutex add more synchronization overhead than OpenMP's optimized runtime
- **At larger matrices (N=2000)**, Pthreads becomes competitive, suggesting block-based LU would benefit both approaches
- **For quick parallelization**, OpenMP is the clear winner with minimal code changes required

## Building & Running

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install gcc libomp-dev

# macOS
brew install gcc libomp
```

### Compile
```bash
# Sequential
gcc -O3 -march=native -o lu_seq sequential.c -lm

# OpenMP
gcc -O3 -march=native -fopenmp -o lu_omp openmp.c -lm

# Pthreads
gcc -O3 -march=native -pthread -o lu_pthread pthreads.c -lm
```

### Run
```bash
# Sequential (N = matrix size)
./lu_seq 1000

# OpenMP (N = matrix size, T = thread count)
./lu_omp 1000 8

# Pthreads (N = matrix size, T = thread count)
./lu_pthread 1000 4
```

## Interactive Demo

Try the [live visualization](https://vednat123.github.io/mppproject/) to see LU decomposition in action:

- Step-by-step animation of the algorithm
- Adjustable matrix sizes (3×3 to 5×5)
- Visual highlighting of pivot selection and row elimination
- Speed controls for detailed study

## Project Structure

```
mppproject/
├── sequential.c      # Baseline sequential implementation
├── openmp.c          # OpenMP parallel version
├── pthreads.c        # Pthreads parallel version
├── index.html        # Interactive web demo
└── README.md
```

## Future Work

- Implement block-based LU decomposition for better cache utilization
- Test on multi-socket NUMA systems
- Add GPU acceleration using CUDA or OpenCL
- Compare with optimized LAPACK implementations

## Author

**Vedant Trivedi**  
Ontario Tech University  
[GitHub](https://github.com/vednat123) • [LinkedIn](https://linkedin.com/in/vedant-trivedi-a8445a268)

## License
Feel free to use this code for educational purposes.
