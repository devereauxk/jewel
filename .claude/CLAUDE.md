# JEWEL Workspace

Monte Carlo event generation for Z+jet production in pp and pPb collisions using JEWEL (Jet Evolution With Energy Loss).

## Directory Structure

```
jewel_workspace/
├── jewel-2.4.0-2D/    Main workspace: vacuum + 2D hydro medium executables (unmodified upstream)
├── jewel-2.4.0-2D-MOD/ Modified JEWEL 2.4.0 with PPZJ bug fixes (see CHANGES.diff)
├── jewel-2.2.0/       Older JEWEL version (vacuum only, for cross-version validation)
├── convert/           HepMC→ROOT converter (ConvertHepMCToRoot)
├── validation/        Validation/comparison tools, run scripts, plots, ROOT files
├── hydro/sample/      100 Ncoll bins of Trajectum hydro profiles (8.16 TeV pPb)
├── lhapdf/            LHAPDF 6.5.5 (lib/ and share/LHAPDF/)
├── local_deps/        System libraries needed at runtime (libpcre, etc.)
├── jewel_pp-v9.root   External reference (2M events, pp 5020 GeV)
└── jewel_pp-v7.root   Older external reference (100k events)
```

## Build

### JEWEL executables

```bash
cd jewel-2.4.0-2D/
make            # builds jewel-2.4.0-vac, jewel-2.4.0-simple, jewel-2.4.0-2D

cd jewel-2.4.0-2D-MOD/
make            # same executables, with PPZJ bug fixes
```

Fortran source, links against LHAPDF at `lhapdf/lib/`. The MOD build should be used for all PPZJ physics analysis — the unmodified 2.4.0 has zero parton shower evolution for Z/W+jet processes.

### Converter

```bash
cd convert/
make            # builds ConvertHepMCToRoot
```

### Validation tools

```bash
cd validation/
make            # builds ValidateJewel, ValidateJewel3Way, ValidateJewelPPb, CompareTrackPt
```

Both use C++ with ROOT 6.34.04 at `/raid5/root/root-v6.34.04/root/`.

## Environment

Before running JEWEL:
```bash
cd jewel-2.4.0-2D/ && source setup.sh      # unmodified
cd jewel-2.4.0-2D-MOD/ && source setup.sh   # modified (preferred for PPZJ)
```

Before running conversion/validation tools:
```bash
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}
```

## Event Generation

### Config templates (`jewel-2.4.0-2D/korinna/`)

| Template | System | Energy | PTMIN | WEXPO | Executable |
|----------|--------|--------|-------|-------|------------|
| `ZJet_pp.dat` | pp vacuum | 5020 GeV | 15 | 4.5 | jewel-2.4.0-vac |
| `ZJet_pp_8160.dat` | pp vacuum | 8160 GeV | 15 | 4.5 | jewel-2.4.0-vac |
| `ZJet_pp8160v2.dat` | pp vacuum | 8160 GeV | 5 | 1.2 | jewel-2.4.0-vac |
| `ZJet_pp8160v3.dat` | pp vacuum | 8160 GeV | 0 | 1.4 | jewel-2.4.0-vac |
| `ZJet_pp8160_wexpo4p5.dat` | pp vacuum | 8160 GeV | 0 | 4.5 | jewel-2.4.0-vac |
| `ZJet_pPb.dat` | pPb hydro | 8160 GeV | 15 | 4.5 | jewel-2.4.0-2D |
| `ZJet_pPb_v2.dat` | pPb hydro | 8160 GeV | 0 | 1.4 | jewel-2.4.0-2D |

JEWEL 2.2.0 templates in `jewel-2.2.0/korinna/`:

| Template | System | Energy | PTMIN | WEXPO |
|----------|--------|--------|-------|-------|
| `ZJet_pp.dat` | pp vacuum | 5020 GeV | 15 | 4.5 |
| `ZJet_pp_v2.dat` | pp vacuum | 5020 GeV | 0 | 1.4 |

JEWEL 2.4.0-2D-MOD templates in `jewel-2.4.0-2D-MOD/korinna/`:

| Template | System | Energy | PTMIN | WEXPO | Recoils | Executable |
|----------|--------|--------|-------|-------|---------|------------|
| `ZJet_pp8160v3.dat` | pp vacuum | 8160 GeV | 0 (unclamped) | 1.4 | n/a | jewel-2.4.0-vac |
| `ZJet_pPb_v2.dat` | pPb hydro | 8160 GeV | 0 (unclamped) | 1.4 | on | jewel-2.4.0-2D |
| `ZJet_pPb_v3.dat` | pPb hydro | 8160 GeV | 0 (unclamped) | 1.4 | off | jewel-2.4.0-2D |
| `ZJet_pPb_v4.dat` | pPb hydro | 8160 GeV | 0 (unclamped) | 1.4 | on | jewel-2.4.0-2D |
| `ZJet_pp8160v4.dat` | pp vacuum | 8160 GeV | 15 | 1.5 | n/a | jewel-2.4.0-vac |

Placeholders: `xxxx` → job/bin ID, `yyyy` → events per bin (pPb only).

### Generation scripts (`jewel-2.4.0-2D/`)

| Script | System | Events | Config | Parallelism |
|--------|--------|--------|--------|-------------|
| `genPPZJet.sh` | pp 5020 | 100k (100 x 1000) | `ZJet_pp.dat` | 5 parallel |
| `genPP8160ZJet.sh` | pp 8160 | 2M (2000 x 1000) | `ZJet_pp_8160.dat` | 10 parallel |
| `genPP8160v2ZJet.sh` | pp 8160 | 500k (500 x 1000) | `ZJet_pp8160v2.dat` | 5 parallel |
| `genPP8160v3ZJet.sh` | pp 8160 | 500k (500 x 1000) | `ZJet_pp8160v3.dat` | 5 parallel |
| `genPP8160_wexpo4p5_ZJet.sh` | pp 8160 | 500k (500 x 1000) | `ZJet_pp8160_wexpo4p5.dat` | 5 parallel |
| `genPPbZJet.sh` | pPb 8160 | 100k (100 bins x 1000) | `ZJet_pPb.dat` | 5 parallel |
| `genPPb2MZJet.sh` | pPb 8160 | 2M (100 bins x 20000) | `ZJet_pPb.dat` | 10 parallel |
| `genPPbv2ZJet.sh` | pPb 8160 | 500k (100 bins x 5000) | `ZJet_pPb_v2.dat` | 5 parallel |

JEWEL 2.4.0-2D-MOD scripts in `jewel-2.4.0-2D-MOD/`:

| Script | System | Events | Config | Parallelism |
|--------|--------|--------|--------|-------------|
| `genPP8160v3ZJet.sh` | pp 8160 | 500k (500 x 1000) | `ZJet_pp8160v3.dat` | 5 parallel |
| `genPPbv2ZJet.sh` | pPb 8160 (recoils on) | 500k (100 bins x 5000) | `ZJet_pPb_v2.dat` | 5 parallel |
| `genPPbv3ZJet.sh` | pPb 8160 (recoils off) | 500k (100 bins x 5000) | `ZJet_pPb_v3.dat` | 5 parallel |
| `genPPbv4ZJet.sh` | pPb 8160 (recoils on) | ~500k (57 unique profiles, mult×Ncoll-weighted) | `ZJet_pPb_v4.dat` | 5 parallel |
| `genPP8160v4ZJet.sh` | pp 8160 | 500k (500 x 1000) | `ZJet_pp8160v4.dat` | 5 parallel |

JEWEL 2.2.0 scripts in `jewel-2.2.0/`:

| Script | Events | Config |
|--------|--------|--------|
| `genPPZJet.sh` | 2M (2000 x 1000) | `ZJet_pp.dat` |
| `genPPv2ZJet.sh` | 500k (500 x 1000) | `ZJet_pp_v2.dat` |

### Key physics parameters

Common to all runs:
- PROCESS PPZJ, ISOCHANNEL PP, HADRO T
- PDFSET 10042 (nCTEQ15 / cteq6l1), PTMAX 1200
- MASS 1, NPROTON 1 (2.4.0) or NSET 0 (2.2.0) — no nuclear PDF
- pPb with recoils: KEEPRECOILS T, COMPRESS T, WRITESCATCEN T, WRITEDUMMIES T
- pPb without recoils: KEEPRECOILS F (no hole subtraction needed)

PTMIN and WEXPO vary by sample — see config templates table above. WEXPO controls importance sampling via `pT_hat^WEXPO`; all events carry EventWeight to compensate. WEXPO=1.4 with PTMIN=0 gives roughly equal statistics in Z pT bins [0,30) and [30,500] GeV (see `wexpo_study.md` in the working install).

**PTMIN clamp:** Unmodified JEWEL 2.4.0 and 2.2.0 clamp PTMIN to a minimum of 3 GeV (line 575 of `jewel-2.4.0.f`). All PTMIN=0 configs in unmodified builds actually run at PTMIN=3. The 2.4.0-2D-MOD build removes this clamp for Z/W+jet processes (where the Z mass regulates the cross section). Files marked "unclamped" in the produced ROOT files table use the true PTMIN=0.

**MSTP(125)=2 bug:** Unmodified JEWEL 2.4.0 sets `MSTP(125)=2` which disables the parton shower for all non-PPJJ processes (zero splittings, zero medium scatterings). The 2.4.0-2D-MOD build fixes this. See `jewel-2.4.0-2D-MOD/CHANGES.diff` for details.

**PICKVTX bug (medium-2D.f):** The hard-scattering vertex sampler indexed `ncollNZ` (non-zero cells) using `rndIndex = int(pyr(0) * ncollSum)`, but `ncollSum` (total binary collisions) exceeds the valid `ncollNZ` range. Out-of-range indices hit zero-initialized memory, pinning 5-27% of vertices at (0,0) — the fireball center. The fix uses the correct `ncollpDist` array (one row per binary collision, already built by `READNCOLL` but previously dead code) with 1-based indexing.

### Hydro profiles

Located at `hydro/sample/` with 100 directories (57 unique Trajectum minimum-bias events; 11 triplicated, 21 duplicated). Directory names are `<batchPrefix>.<eventNumber>` from the extraction script, not physics quantities. Some have dot-prefixed names (e.g., `.10`, `.13`) — **must use `find -L`** to discover all. Hardlinked into `jewel-2.4.0-2D/hydro/pPb/sample/` and `jewel-2.4.0-2D-MOD/hydro/pPb/sample/`. Ncoll ranges from 1 to 21 across unique events (total 532 over unique profiles). The v4 generation script runs each unique profile once, with events weighted proportional to slot multiplicity × Ncoll.

Each bin contains: `NCollHisto.dat`, `Tcontour*` (29 time slices), `Vcontour*` (29 time slices).

## Conversion Pipeline

### HepMC → ROOT (`convert/`)

```bash
convert/ConvertHepMCToRoot input1.hepmc [input2.hepmc ...] output.root
convert/ConvertHepMCToRoot --NegativeID 3 input1.hepmc [input2.hepmc ...] output.root
```

Last argument is always the output. Shell globs work for input files.
`--NegativeID 3` maps HepMC status-3 particles (scattering centres from WRITESCATCEN) to `trackWeight = -1` for hole subtraction in medium runs.

### ROOT tree structure

- Tree name: **`Tree`** (not "JewelTree")
- Per-event: `EventWeight`, `genZPt`/`genZEta`/`genZY`/`genZPhi`/`genZMass` (vectors)
- Per-event: `genMuPt1`/`Pt2`/`Eta1`/`Eta2`/`Phi1`/`Phi2`
- Per-track: `trackPt`/`Eta`/`Phi`/`Y`/`PDFId`/`Weight`/`ResidualWeight`/`Charge` (vectors)

### Track selection (in validation tools)

- `trackWeight >= 0.5` (pp) or `trackWeight != 0` (pPb with recoils)
- `trackCharge != 999` (not neutral placeholder)
- `|eta| < 2.4`
- `pT > 0.5 GeV`

### Hole subtraction (pPb with KEEPRECOILS T)

- `trackWeight` values: +1 (final-state), 0 (intermediate/neutral), -1 (scattering centre holes)
- Standard JEWEL prescription: fill weight = `trackWeight * EventWeight` (holes subtract)
- FHead analysis uses a reduced subtraction: `trackWeight * EventWeight * (1 - 0.33 * (trackWeight < 0))`, giving holes 67% weight. Rationale: JEWEL holes are partonic (not hadronized), so full subtraction over-subtracts since one parton would fragment into ~3 hadrons.
- Z histograms use EventWeight only (no hole correction)
- pPb with KEEPRECOILS F has no holes — use `EventWeight` for all tracks

## Validation Tools (`validation/`)

| Tool | Args | Purpose |
|------|------|---------|
| `ValidateJewel` | `sample.root ref.root outDir/` | 2-way shape comparison |
| `ValidateJewel3Way` | `jewel240.root jewel220.root ref.root outDir/` | 3-way version comparison |
| `ValidateJewelPPb` | `pPb.root pp.root outDir/` | pPb vs pp with ratio panels |

Run scripts in `validation/` (e.g., `run_all_2M.sh`, `run_validation_pPb.sh`) handle conversion + validation end-to-end, referencing the converter at `../convert/ConvertHepMCToRoot`.

### Normalization

- **3-way pp (ValidateJewel3Way):** Shape normalization via `Integral("width")` — pure shape comparison.
- **pPb vs pp (ValidateJewelPPb):** Produces two variants:
  - `_uwNorm`: divides by unweighted Z count (correct for cross-section comparison)
  - `_wNorm`: divides by weighted Z count (incorrect, produces constant offset)
  - **Use `_uwNorm` results.**

## Plot Directory Naming

Pattern: `plots_{comparison}_{energy}_{stats}/`

Examples:
- `plots_3way_pp5020_2M/` — 3-way pp comparison at 5020 GeV, 2M events
- `plots_pPb_vs_pp8160_2M/` — pPb vs pp at 8160 GeV, 2M events
- `plots_pPb_vs_pp8160_100k/` — same comparison at 100k

## Produced ROOT Files (`validation/`)

| File | JEWEL | System | Energy | PTMIN | WEXPO | Recoils | Events |
|------|-------|--------|--------|-------|-------|---------|--------|
| `jewel_pp_220.root` | 2.2.0 | pp | 5020 | 15 | 4.5 | n/a | 2,000,000 |
| `jewel_pp_220_v2_500k.root` | 2.2.0 | pp | 5020 | 0 | 1.4 | n/a | 500,000 |
| `jewel_pp_240.root` | 2.4.0 | pp | 5020 | 15 | 4.5 | n/a | 1,886,306 |
| `jewel_pp8160_2M.root` | 2.4.0 | pp | 8160 | 15 | 4.5 | n/a | 1,831,529 |
| `jewel_pp8160v2_500k.root` | 2.4.0 | pp | 8160 | 5 | 1.2 | n/a | 468,857 |
| `jewel_pp8160v3_500k.root` | 2.4.0 | pp | 8160 | 0 | 1.4 | n/a | 468,816 |
| `jewel_pp8160_wexpo4p5_500k.root` | 2.4.0 | pp | 8160 | 0 | 4.5 | n/a | 472,273 |
| `jewel_pPb_2M.root` | 2.4.0-2D | pPb | 8160 | 15 | 4.5 | on | 1,340,059 |
| `jewel_pPb_v2_500k.root` | 2.4.0-2D | pPb | 8160 | 0 | 1.4 | on | 468,584 |
| `jewel_pp8160v3_MOD_500k.root` | 2.4.0-2D-MOD | pp | 8160 | 0 (unclamped) | 1.4 | n/a | 500,000 |
| `jewel_pPb_v2_MOD_500k.root` | 2.4.0-2D-MOD | pPb | 8160 | 0 (unclamped) | 1.4 | on | 500,000 |
| `jewel_pPb_v3_MOD_norecoil_500k.root` | 2.4.0-2D-MOD | pPb | 8160 | 0 (unclamped) | 1.4 | off | 500,000 |
| `jewel_pPb_v4_MOD_500k.root` | 2.4.0-2D-MOD | pPb | 8160 | 0 (unclamped) | 1.4 | on | ~500,000 |
| `jewel_pp8160v4_MOD_500k.root` | 2.4.0-2D-MOD | pp | 8160 | 15 | 1.5 | n/a | 500,000 |

Unmodified 2.4.0/2.4.0-2D samples have no parton shower evolution for PPZJ and should not be used for physics analysis. All PTMIN=0 entries without "(unclamped)" were actually PTMIN=3 due to the clamp bug. The v4 sample additionally includes the PICKVTX vertex fix and runs each of the 57 unique hydro profiles once, with events allocated proportional to slot multiplicity × Ncoll (duplicate directories count as extra minimum-bias slots).

Reference files (external, in workspace root): `jewel_pp-v9.root` (2M events, pp 5020 GeV), `jewel_pp-v7.root` (100k events, pp 5020 GeV). Generation settings unknown.

## Common Issues

- ROOT needs `local_deps/lib/x86_64-linux-gnu` in LD_LIBRARY_PATH for libpcre
- WEXPO 4.5 gives ~94% acceptance for pp vacuum (1000 requested → ~940 good events)
- pPb with Trajectum 8.16 TeV hydro gives ~94% acceptance. With the MOD build, pPb produces ~19 scatterings/event and ~2.5 splittings/event. The unmodified 2.4.0 showed 0 scatterings due to the MSTP(125)=2 bug
- The snap-installed ROOT (`/snap/root-framework/`) conflicts with the local install — always set PATH/LD_LIBRARY_PATH explicitly
- HepMC conversion from 2000 files uses shell glob expansion (~140 KB args, within ARG_MAX)
