#!/usr/bin/env bash
set -euo pipefail
source setup.sh
mkdir -p parameters logs eventfiles

TOTAL_EVENTS=100000
HYDRO_SAMPLE="hydro/pPb/sample"
TEMPLATE="korinna/ZJet_pPb.dat"
MAX_PARALLEL=5

bins=()
while IFS= read -r d; do
    bins+=("$(basename "$d")")
done < <(find -L "$HYDRO_SAMPLE" -mindepth 1 -maxdepth 1 -type d | sort)

NBINS=${#bins[@]}
EVENTS_PER_BIN=$((TOTAL_EVENTS / NBINS))
REMAINDER=$((TOTAL_EVENTS - EVENTS_PER_BIN * NBINS))

echo "Found $NBINS Ncoll bins, $EVENTS_PER_BIN events per bin ($REMAINDER extra spread to first bins)"

count=0
for i in "${!bins[@]}"; do
    bin="${bins[$i]}"
    nevt=$EVENTS_PER_BIN
    [[ $i -lt $REMAINDER ]] && nevt=$((nevt + 1))

    # Create medium params file for this bin
    cat > "parameters/medium_pPb_${bin}.dat" <<MEDEOF
CENTRMIN 0.
CENTRMAX 100.
BMIN 0.
BMAX 4.93
A 208
HYDRODIR ${HYDRO_SAMPLE}/${bin}
NCOLLHISTO ${HYDRO_SAMPLE}/${bin}/NCollHisto.dat
MEDEOF

    # Create JEWEL params file from template
    sed -e "s/xxxx/$bin/g" -e "s/yyyy/$nevt/g" "$TEMPLATE" > "parameters/ZJet_pPb_${bin}.dat"

    echo "[$((i+1))/$NBINS] Launching bin $bin ($nevt events)..."
    ./jewel-2.4.0-2D "parameters/ZJet_pPb_${bin}.dat" &

    count=$((count + 1))
    if [[ $((count % MAX_PARALLEL)) -eq 0 ]]; then
        wait
    fi
done
wait
echo "All $NBINS bins complete. Total events: $TOTAL_EVENTS"
