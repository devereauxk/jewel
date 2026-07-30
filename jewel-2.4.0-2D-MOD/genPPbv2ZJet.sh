#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
source setup.sh
mkdir -p parameters logs eventfiles

TOTAL_EVENTS=500000
HYDRO_SAMPLE="hydro/pPb/sample"
TEMPLATE="korinna/ZJet_pPb_v2.dat"
MAX_PARALLEL=5

echo "Generating ${TOTAL_EVENTS} pPb 8160 GeV events with JEWEL 2.4.0-MOD + 2D hydro"
echo "PTMIN=0, PTMAX=1200, WEXPO=1.4"
echo "Started: $(date)"

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

    cat > "parameters/medium_pPb_${bin}.dat" <<MEDEOF
CENTRMIN 0.
CENTRMAX 100.
BMIN 0.
BMAX 4.93
A 208
HYDRODIR ${HYDRO_SAMPLE}/${bin}
NCOLLHISTO ${HYDRO_SAMPLE}/${bin}/NCollHisto.dat
MEDEOF

    sed -e "s/xxxx/$bin/g" -e "s/yyyy/$nevt/g" "$TEMPLATE" > "parameters/ZJet_pPb_v2_${bin}.dat"

    echo "[$((i+1))/$NBINS] Launching bin $bin ($nevt events)..."
    ./jewel-2.4.0-2D "parameters/ZJet_pPb_v2_${bin}.dat" &

    count=$((count + 1))
    if [[ $((count % MAX_PARALLEL)) -eq 0 ]]; then
        wait
        echo "  Completed $count / $NBINS bins ($(date))"
    fi
done
wait
echo "All $NBINS bins complete. $(date)"

echo ""
echo "=== Converting HepMC to ROOT ==="
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}
CONVERTDIR="/raid5/data/kdevero/jewel_workspace/convert"
OUTROOT="/raid5/data/kdevero/jewel_workspace/validation/jewel_pPb_v2_MOD_500k.root"

"$CONVERTDIR/ConvertHepMCToRoot" --NegativeID 3 eventfiles/out_ZJet_pPb_v2_*.hepmc "$OUTROOT"
echo "ROOT file: $OUTROOT"
echo "Done. $(date)"
