#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
source setup.sh
mkdir -p parameters logs eventfiles

TEMPLATE=korinna/ZJet_pp.dat

for w in 0 1 2 3 4; do
    sed "s/xxxx/wexpo_${w}/g; s/NEVENT 1000/NEVENT 5000/; s/PTMIN 15\./PTMIN 5./; s/PTMAX 1200\./PTMAX 600./; s/WEXPO 4.5/WEXPO ${w}.0/" \
        "$TEMPLATE" > "parameters/ZJet_pp_wexpo_${w}.dat"
    ./jewel-2.4.0-vac "parameters/ZJet_pp_wexpo_${w}.dat" &
done
wait

echo ""
echo "WEXPO  N_Z    N(0-30) N(30-500) ratio(low/high)"
echo "-----  -----  ------- --------- ---------------"
for w in 0 1 2 3 4; do
    printf "%-6s " "$w"
    python3 count_zpt.py "eventfiles/out_ZJet_pp_wexpo_${w}.hepmc"
done
