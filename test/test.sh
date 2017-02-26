#!/bin/bash

set -xue

python3 gen_long_ped.py > long_ped.csv
echo "1" > list

time python3 ../pedRefiner.py list long_ped.csv out.long_ped.csv
