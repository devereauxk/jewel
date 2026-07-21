#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
source setup.sh

STUDYDIR="study_trackpt"
TEMPLATE="korinna/ZJet_pp.dat"
MAX_PARALLEL=10
NJOBS=100

for variant in B C D E F; do
    echo "=== Running variant $variant ==="
    count=0
    for (( c=1; c<=NJOBS; c++ )); do
        paramfile="$STUDYDIR/parameters/ZJet_pp_${variant}_${c}.dat"
        sed -e "s/xxxx/${variant}_${c}/g" "$TEMPLATE" > "$paramfile"
        # Override log/hepmc paths to study directory
        sed -i "s|logs/out_ZJet_pp_${variant}_${c}.log|$STUDYDIR/logs/out_${variant}_${c}.log|" "$paramfile"
        sed -i "s|eventfiles/out_ZJet_pp_${variant}_${c}.hepmc|$STUDYDIR/eventfiles/out_${variant}_${c}.hepmc|" "$paramfile"

        "$STUDYDIR/bin/jewel-vac-$variant" "$paramfile" &

        count=$((count + 1))
        [[ $((count % MAX_PARALLEL)) -eq 0 ]] && wait
    done
    wait
    echo "Variant $variant complete: $(ls $STUDYDIR/eventfiles/out_${variant}_*.hepmc 2>/dev/null | wc -l) files"
done

echo "All variants complete."
