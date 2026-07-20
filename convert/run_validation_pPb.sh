#!/usr/bin/env bash
set -euo pipefail
CONVERTDIR="$(cd "$(dirname "$0")" && pwd)"
JEWELDIR="/raid5/data/kdevero/jewel_workspace/jewel-2.4.0-2D"
PPFILE="$CONVERTDIR/jewel_pp_240.root"
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}

echo "=== Step 1: Convert pPb HepMC to ROOT ==="
"$CONVERTDIR/ConvertHepMCToRoot" "$JEWELDIR"/eventfiles/out_ZJet_pPb_*.hepmc "$CONVERTDIR/jewel_pPb_240.root"

echo ""
echo "=== Step 2: Run pPb vs pp comparison ==="
mkdir -p "$CONVERTDIR/plots_pPb"
"$CONVERTDIR/ValidateJewelPPb" "$CONVERTDIR/jewel_pPb_240.root" "$PPFILE" "$CONVERTDIR/plots_pPb/"

echo ""
echo "Done. Plots in $CONVERTDIR/plots_pPb/"
