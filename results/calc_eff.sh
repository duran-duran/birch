#!/bin/bash

PAR=$1
SEQ=$2

echo "# CPUS N Efficiency"

for n in $(seq 8 8 80)
do
  cat $PAR | grep " $n " |
  while read l
  do
    let N=$(echo $l | awk '{print $2}')
    let PAR_TIME=$(echo $l | awk '{print $3}')
    let SEQ_TIME=$(cat $SEQ | grep $N | awk '{print $3}')
    EFF=$(echo "scale=2;100*$SEQ_TIME/$PAR_TIME/$n" | bc -l)
    echo $n $N $EFF
  done
done
