#!/usr/bin/env bash
##############################################################################
# stress_test_seq.sh
#
# Purpose:
#   1. Compile your sequential LU program (sequential.c).
#   2. Run multiple tests for different matrix sizes.
#   3. Generate random matrix input automatically (so no manual typing needed).
#   4. Capture and display timing results.
#
# Usage:
#   chmod +x stress_test_seq.sh
#   ./stress_test_seq.sh
#
# Notes:
#   - Adjust the program name, compiler flags, matrix sizes, random range,
#     and number of repeats as needed.
#   - Increase MAX_LINE in your code if you’re testing large matrices.
##############################################################################

# ----- CONFIGURABLE SETTINGS -----

# Your sequential source file & output executable name
SRC_FILE="sequential.c"
EXEC_NAME="sequential_lu"

# Compiler and flags (no OpenMP needed here)
CC="gcc"
CFLAGS="-O3"

# List of matrix sizes to test
SIZES=(100 200 300 400 500)

# Range of random values. We'll generate them between -9 and +9
MIN_VAL=-9
MAX_VAL=9

# Number of times to run each size (to measure average timing)
REPEATS=3


# ----- COMPILE -----

echo "Compiling $SRC_FILE => $EXEC_NAME with flags: $CFLAGS"
$CC $SRC_FILE -o $EXEC_NAME $CFLAGS
if [[ $? -ne 0 ]]; then
  echo "Compilation failed. Exiting."
  exit 1
fi
echo "Compilation successful."
echo


# ----- STRESS TEST LOOP -----
echo "Starting stress test for sequential LU ..."
echo

for SIZE in "${SIZES[@]}"; do
  echo "====================================="
  echo "Matrix size: $SIZE x $SIZE"
  echo "====================================="

  # Run multiple repeats
  for ((r=1; r<=REPEATS; r++)); do
    
    echo "Run #$r:"
    # We'll time the run using /usr/bin/time
    /usr/bin/time -f "Elapsed time: %E (hh:mm:ss), CPU: %P" \
    ./$EXEC_NAME <<EOF
$SIZE
$( for ((i=0; i<$SIZE; i++)); do
     for ((j=0; j<$SIZE; j++)); do
       # Generate integer random number in [MIN_VAL, MAX_VAL]
       RANDVAL=$(( (RANDOM % ($MAX_VAL - $MIN_VAL + 1)) + $MIN_VAL ))
       echo -n "$RANDVAL "
     done
     echo
   done
)
n
EOF
    # The program reads "n" for "Verify decomposition? (y/n)"
    # so we answer "n" automatically to skip verification.

    echo
  done

done

echo "Sequential stress test complete."
