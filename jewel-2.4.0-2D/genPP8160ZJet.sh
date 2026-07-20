#!/usr/bin/env bash
set -euo pipefail
source setup.sh
mkdir -p parameters logs eventfiles

MAX_PARALLEL=10
count=0
for (( c=1; c<=2000; c++ )); do
    sed "s/xxxx/$c/g" korinna/ZJet_pp_8160.dat > parameters/ZJet_pp8160_$c.dat
    ./jewel-2.4.0-vac parameters/ZJet_pp8160_$c.dat &
    count=$((count + 1))
    [[ $((count % MAX_PARALLEL)) -eq 0 ]] && wait
done
wait
echo "All 2000 pp 8160 jobs complete."
