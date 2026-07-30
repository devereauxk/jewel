#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
source setup.sh
mkdir -p parameters logs eventfiles

TEMPLATE=korinna/ZJet_pp.dat

for w in 0.8 1.0 1.2 1.4 1.6; do
    tag=$(echo "$w" | tr '.' 'p')
    sed "s/xxxx/wexfine_${tag}/g; s/NEVENT 1000/NEVENT 5000/; s/PTMIN 15\./PTMIN 5./; s/PTMAX 1200\./PTMAX 600./; s/WEXPO 4.5/WEXPO ${w}/" \
        "$TEMPLATE" > "parameters/ZJet_pp_wexfine_${tag}.dat"
    ./jewel-2.4.0-vac "parameters/ZJet_pp_wexfine_${tag}.dat" &
done
wait

echo ""
echo "WEXPO  N_Z    N(0-30) N(30-500) ratio(low/high)"
echo "-----  -----  ------- --------- ---------------"
for w in 0.8 1.0 1.2 1.4 1.6; do
    tag=$(echo "$w" | tr '.' 'p')
    printf "%-6s " "$w"
    python3 count_zpt.py "eventfiles/out_ZJet_pp_wexfine_${tag}.hepmc"
done
