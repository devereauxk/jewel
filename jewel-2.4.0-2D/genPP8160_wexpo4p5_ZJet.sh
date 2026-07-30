#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
source setup.sh
mkdir -p parameters logs eventfiles

TEMPLATE=korinna/ZJet_pp8160_wexpo4p5.dat
NJOBS=500
PARALLEL=5

echo "Generating 500k pp 8160 GeV events (${NJOBS} jobs x 1000, ${PARALLEL} parallel)"
echo "PTMIN=0, PTMAX=1200, WEXPO=4.5"
echo "Started: $(date)"

count=0
for (( c=1; c<=NJOBS; c++ )); do
    sed "s/xxxx/$c/g" "$TEMPLATE" > "parameters/ZJet_pp8160_wexpo4p5_$c.dat"
    ./jewel-2.4.0-vac "parameters/ZJet_pp8160_wexpo4p5_$c.dat" &
    count=$((count + 1))
    if [[ $((count % PARALLEL)) -eq 0 ]]; then
        wait
        echo "  Completed $count / $NJOBS jobs ($(date))"
    fi
done
wait
echo "All $NJOBS jobs complete. $(date)"

echo ""
echo "=== Converting HepMC to ROOT ==="
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}
CONVERTDIR="/raid5/data/kdevero/jewel_workspace/convert"
OUTROOT="/raid5/data/kdevero/jewel_workspace/validation/jewel_pp8160_wexpo4p5_500k.root"

"$CONVERTDIR/ConvertHepMCToRoot" eventfiles/out_ZJet_pp8160_wexpo4p5_*.hepmc "$OUTROOT"
echo "ROOT file: $OUTROOT"
echo "Done. $(date)"
