#!/bin/bash

OUTFILE="results.txt"

SIZES=(500 1000 1500 2000)

THREAD_COUNTS=(1 2 4 8 16)


SEQ_EXE="./sequential/lu_seq"
OMP_EXE="./omp_ver/lu_omp"
PTH_EXE="./posix_ver/lu_pthreads"
OCL_EXE="./openCL_ver/lu_opencl"

if [ -f "$OUTFILE" ]; then
    rm "$OUTFILE"
fi

echo "====================================" | tee -a "$OUTFILE"
echo "   LU Decomposition Full Test Suite   " | tee -a "$OUTFILE"
echo "====================================" | tee -a "$OUTFILE"
echo "" | tee -a "$OUTFILE"

echo "---- Sequential Version ----" | tee -a "$OUTFILE"
for size in "${SIZES[@]}"; do
    echo "Matrix Size = $size" | tee -a "$OUTFILE"
    $SEQ_EXE $size 2>&1 | tee -a "$OUTFILE"
    echo "" | tee -a "$OUTFILE"
done

echo "---- OpenMP Version ----" | tee -a "$OUTFILE"
for size in "${SIZES[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        echo "Matrix Size = $size, Threads = $threads" | tee -a "$OUTFILE"
        $OMP_EXE $size $threads 2>&1 | tee -a "$OUTFILE"
        echo "" | tee -a "$OUTFILE"
    done
done


echo "---- Pthreads Version ----" | tee -a "$OUTFILE"
for size in "${SIZES[@]}"; do
    for threads in "${THREAD_COUNTS[@]}"; do
        echo "Matrix Size = $size, Threads = $threads" | tee -a "$OUTFILE"
        $PTH_EXE $size $threads 2>&1 | tee -a "$OUTFILE"
        echo "" | tee -a "$OUTFILE"
    done
done

echo "All tests completed. Results in $OUTFILE."
