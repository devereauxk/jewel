#!/usr/bin/env bash
set -euo pipefail
VALDIR="$(cd "$(dirname "$0")" && pwd)"
CONVERTDIR="$VALDIR/../convert"
JEWELDIR="/raid5/data/kdevero/jewel_workspace/jewel-2.4.0-2D"
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}

echo "=== Step 1: Convert pp 8160 2M HepMC to ROOT ==="
"$CONVERTDIR/ConvertHepMCToRoot" "$JEWELDIR"/eventfiles/out_ZJet_pp8160_*.hepmc "$VALDIR/jewel_pp8160_2M.root"

echo ""
echo "=== Step 2: Convert pPb 2M HepMC to ROOT ==="
"$CONVERTDIR/ConvertHepMCToRoot" --NegativeID 3 "$JEWELDIR"/eventfiles/out_ZJet_pPb_*.hepmc "$VALDIR/jewel_pPb_2M.root"

echo ""
echo "=== Step 3: pPb vs pp comparison at 8160 GeV (2M events) ==="
mkdir -p "$VALDIR/plots_pPb_vs_pp8160_2M"
"$VALDIR/ValidateJewelPPb" "$VALDIR/jewel_pPb_2M.root" "$VALDIR/jewel_pp8160_2M.root" "$VALDIR/plots_pPb_vs_pp8160_2M/"

echo ""
echo "=== Step 4: 3-way pp comparison at 5020 GeV (2M events) ==="
mkdir -p "$VALDIR/plots_3way_pp5020_2M"
"$VALDIR/ValidateJewel3Way" "$VALDIR/jewel_pp_240.root" "$VALDIR/jewel_pp_220.root" /raid5/data/kdevero/jewel_workspace/jewel_pp-v9.root "$VALDIR/plots_3way_pp5020_2M/"

echo ""
echo "Done. Plots in:"
echo "  $VALDIR/plots_pPb_vs_pp8160_2M/"
echo "  $VALDIR/plots_3way_pp5020_2M/"
