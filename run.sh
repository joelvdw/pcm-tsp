#!/bin/bash
set -e

NAME=no_opti
FILE=dj16.tsp
THS="64 128 256"
# s=`seq 10 10 256`
# THS="1 $s 64 128 256"

for t in $THS
do
  echo "${FILE%*.tsp}-$t"
  ./tsp $FILE $t > "${FILE%*.tsp}-$t.txt"
done