#!/usr/bin/env bash
set -euo pipefail
CONVERTDIR="$(cd "$(dirname "$0")" && pwd)"
JEWELDIR="/raid5/data/kdevero/jewel_workspace/jewel-2.2.0"
REFFILE="/raid5/data/kdevero/jewel_workspace/jewel_pp-v7.root"
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}

echo "=== Step 1: Convert HepMC to ROOT ==="
"$CONVERTDIR/ConvertHepMCToRoot" "$JEWELDIR"/eventfiles/out_ZJet_pp_*.hepmc "$CONVERTDIR/jewel_pp_220.root"

echo ""
echo "=== Step 2: Run validation overlay ==="
mkdir -p "$CONVERTDIR/plots_220"
"$CONVERTDIR/ValidateJewel" "$CONVERTDIR/jewel_pp_220.root" "$REFFILE" "$CONVERTDIR/plots_220/"

echo ""
echo "Done. Plots in $CONVERTDIR/plots_220/"
