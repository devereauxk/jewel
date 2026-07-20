#!/usr/bin/env bash
set -euo pipefail
source setup.sh
mkdir -p parameters logs eventfiles

count=0
for (( c=1; c<=100; c++ )); do
    sed "s/xxxx/$c/g" korinna/ZJet_pp.dat > parameters/ZJet_pp_$c.dat
    ./jewel-2.2.0-vac parameters/ZJet_pp_$c.dat &
    count=$((count + 1))
    [[ $((count % 5)) -eq 0 ]] && wait
done
wait
echo "All 100 jobs complete."
