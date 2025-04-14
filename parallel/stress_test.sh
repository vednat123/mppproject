#!/usr/bin/env bash

SRC_FILE="parallel_lu.c"
EXEC_NAME="parallel_lu"
CC="gcc"
CFLAGS="-O3 -fopenmp"

SIZES=(100 200 300 400 500)
MIN_VAL=-9
MAX_VAL=9
REPEATS=3

echo "Compiling $SRC_FILE => $EXEC_NAME with flags: $CFLAGS"
$CC $SRC_FILE -o $EXEC_NAME $CFLAGS || exit 1
echo "Compilation successful."

for SIZE in "${SIZES[@]}"; do
  echo "====================================="
  echo "Matrix size: $SIZE x $SIZE"
  echo "====================================="

  for ((r=1; r<=REPEATS; r++)); do
    echo "Run #$r:"

    /usr/bin/time -f "Elapsed: %E, CPU: %P" ./$EXEC_NAME <<EOF
$SIZE
$( for ((i=0; i<$SIZE; i++)); do
     for ((j=0; j<$SIZE; j++)); do
       RANDVAL=$(( (RANDOM % ($MAX_VAL - $MIN_VAL + 1)) + $MIN_VAL ))
       echo -n "$RANDVAL "
     done
     echo
   done
)
n
EOF
    echo
  done
done
echo "Done."
