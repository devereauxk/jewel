#!/usr/bin/env bash
set -euo pipefail
CONVERTDIR="$(cd "$(dirname "$0")" && pwd)"
JEWELDIR="/raid5/data/kdevero/jewel_workspace/jewel-2.4.0-2D"
REFFILE="/raid5/data/kdevero/jewel_workspace/jewel_pp-v7.root"
FILE220="$CONVERTDIR/jewel_pp_220.root"
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}

echo "=== Step 1: Convert 2.4.0-2D HepMC to ROOT ==="
"$CONVERTDIR/ConvertHepMCToRoot" "$JEWELDIR"/eventfiles/out_ZJet_pp_*.hepmc "$CONVERTDIR/jewel_pp_240.root"

echo ""
echo "=== Step 2: Run 3-way validation overlay ==="
mkdir -p "$CONVERTDIR/plots_240"
"$CONVERTDIR/ValidateJewel3Way" "$CONVERTDIR/jewel_pp_240.root" "$FILE220" "$REFFILE" "$CONVERTDIR/plots_240/"

echo ""
echo "Done. Plots in $CONVERTDIR/plots_240/"
