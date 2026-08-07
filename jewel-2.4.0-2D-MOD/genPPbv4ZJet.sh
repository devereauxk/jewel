#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
source setup.sh
mkdir -p parameters logs eventfiles

TOTAL_EVENTS=500000
HYDRO_SAMPLE="hydro/pPb/sample"
TEMPLATE="korinna/ZJet_pPb_v4.dat"
MAX_PARALLEL=5

echo "Generating ${TOTAL_EVENTS} pPb 8160 GeV events with JEWEL 2.4.0-MOD + 2D hydro (PICKVTX fix)"
echo "PTMIN=0, PTMAX=1200, WEXPO=1.4, KEEPRECOILS=T"
echo "Deduplicated hydro profiles, events weighted proportional to (slot multiplicity x Ncoll)"
echo "Started: $(date)"

# --- Phase 1: discover unique profiles, their Ncoll, and slot multiplicity ---
# Each of the 100 directories is one equal-probability minimum-bias slot; a
# profile duplicated in k directories fills k slots, so its hard-event weight
# is k * Ncoll.
declare -A md5_to_idx
unique_bins=()
unique_ncoll=()
unique_mult=()

while IFS= read -r d; do
    bin="$(basename "$d")"
    h=$(md5sum "$d/NCollHisto.dat" | awk '{print $1}')
    if [[ -n "${md5_to_idx[$h]+x}" ]]; then
        idx=${md5_to_idx[$h]}
        unique_mult[$idx]=$((unique_mult[idx] + 1))
        echo "  Duplicate slot: $bin (multiplicity of ${unique_bins[$idx]} -> ${unique_mult[$idx]})"
        continue
    fi
    md5_to_idx[$h]=${#unique_bins[@]}
    nc=$(awk 'NR>4 && $3>0 {s+=$3} END {printf "%d",s}' "$d/NCollHisto.dat")
    unique_bins+=("$bin")
    unique_ncoll+=("$nc")
    unique_mult+=(1)
done < <(find -L "$HYDRO_SAMPLE" -mindepth 1 -maxdepth 1 -type d | sort)

NBINS=${#unique_bins[@]}
total_weight=0
total_slots=0
for i in "${!unique_bins[@]}"; do
    total_weight=$((total_weight + unique_mult[i] * unique_ncoll[i]))
    total_slots=$((total_slots + unique_mult[i]))
done
echo ""
echo "Found $NBINS unique hydro profiles filling $total_slots minimum-bias slots"
echo "Total weight sum(mult x Ncoll): $total_weight"
echo ""

# --- Phase 2: allocate events proportional to mult x Ncoll and launch ---
allocated=0
count=0
for i in "${!unique_bins[@]}"; do
    bin="${unique_bins[$i]}"
    nc="${unique_ncoll[$i]}"
    mult="${unique_mult[$i]}"
    nevt=$(( (TOTAL_EVENTS * mult * nc + total_weight / 2) / total_weight ))
    [[ $nevt -lt 1 ]] && nevt=1
    allocated=$((allocated + nevt))

    cat > "parameters/medium_pPb_${bin}.dat" <<MEDEOF
CENTRMIN 0.
CENTRMAX 100.
BMIN 0.
BMAX 4.93
A 208
HYDRODIR ${HYDRO_SAMPLE}/${bin}
NCOLLHISTO ${HYDRO_SAMPLE}/${bin}/NCollHisto.dat
MEDEOF

    sed -e "s/xxxx/$bin/g" -e "s/yyyy/$nevt/g" "$TEMPLATE" > "parameters/ZJet_pPb_v4_${bin}.dat"

    echo "[$((i+1))/$NBINS] Launching bin $bin (Ncoll=$nc, mult=$mult, $nevt events)..."
    ./jewel-2.4.0-2D "parameters/ZJet_pPb_v4_${bin}.dat" &

    count=$((count + 1))
    if [[ $((count % MAX_PARALLEL)) -eq 0 ]]; then
        wait
        echo "  Completed $count / $NBINS profiles ($(date))"
    fi
done
wait
echo "All $NBINS profiles complete. Total events allocated: $allocated. $(date)"

echo ""
echo "=== Converting HepMC to ROOT ==="
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}
CONVERTDIR="/raid5/data/kdevero/jewel_workspace/convert"
OUTROOT="/raid5/data/kdevero/jewel_workspace/validation/jewel_pPb_v4_MOD_500k.root"

"$CONVERTDIR/ConvertHepMCToRoot" --NegativeID 3 eventfiles/out_ZJet_pPb_v4_*.hepmc "$OUTROOT"
echo "ROOT file: $OUTROOT"
echo "Done. $(date)"
