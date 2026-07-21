# JEWEL Workspace

Monte Carlo event generation for Z+jet production in pp and pPb collisions using JEWEL (Jet Evolution With Energy Loss).

## Directory Structure

```
jewel_workspace/
├── jewel-2.4.0-2D/    Main workspace: vacuum + 2D hydro medium executables
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
```

Fortran source, links against LHAPDF at `lhapdf/lib/`.

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
cd jewel-2.4.0-2D/ && source setup.sh
```

Before running conversion/validation tools:
```bash
export LD_LIBRARY_PATH=/raid5/root/root-v6.34.04/root/lib:/raid5/data/kdevero/jewel_workspace/local_deps/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH:-}
```

## Event Generation

### Config templates (`jewel-2.4.0-2D/korinna/`)

| Template | System | Energy | Executable |
|----------|--------|--------|------------|
| `ZJet_pp.dat` | pp vacuum | 5020 GeV | jewel-2.4.0-vac |
| `ZJet_pp_8160.dat` | pp vacuum | 8160 GeV | jewel-2.4.0-vac |
| `ZJet_pPb.dat` | pPb hydro | 8160 GeV | jewel-2.4.0-2D |

Placeholders: `xxxx` → job/bin ID, `yyyy` → events per bin (pPb only).

### Generation scripts (`jewel-2.4.0-2D/`)

| Script | System | Events | Parallelism |
|--------|--------|--------|-------------|
| `genPPZJet.sh` | pp 5020 | 100k (100 jobs x 1000) | 5 parallel |
| `genPP8160ZJet.sh` | pp 8160 | 2M (2000 jobs x 1000) | 10 parallel |
| `genPPbZJet.sh` | pPb 8160 | 100k (100 bins x 1000) | 5 parallel |
| `genPPb2MZJet.sh` | pPb 8160 | 2M (100 bins x 20000) | 10 parallel |

### Key physics parameters (all runs)

- PROCESS PPZJ, ISOCHANNEL PP
- PDFSET 10042 (nCTEQ15)
- WEXPO 4.5 (importance sampling — events carry EventWeight)
- PTMIN 15, PTMAX 1200 GeV
- MASS 1, NPROTON 1
- KEEPRECOILS T, COMPRESS T, WRITESCATCEN T, WRITEDUMMIES T (pPb only)

### Hydro profiles

Located at `hydro/sample/` with 100 Ncoll-bin directories. Some bins have dot-prefixed names (e.g., `.10`, `.13`) — **must use `find -L`** to discover all bins. Symlinked into `jewel-2.4.0-2D/hydro/pPb/sample/`.

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

### Hole subtraction (pPb)

- `trackWeight` values: +1 (final-state), 0 (intermediate/neutral), -1 (scattering centre holes)
- Track fill weight for pPb: `EventWeight * (1 - 0.33 * (trackWeight < 0))`
- Hole tracks get a 0.67 correction factor; normal tracks get 1.0
- Z histograms use EventWeight only (no hole correction)

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

| File | System | Version | Energy | Events |
|------|--------|---------|--------|--------|
| `jewel_pp_220.root` | pp | 2.2.0 | 5020 | 2M |
| `jewel_pp_240.root` | pp | 2.4.0 | 5020 | ~1.89M |
| `jewel_pp8160_2M.root` | pp | 2.4.0 | 8160 | ~2M |
| `jewel_pPb_2M.root` | pPb | 2.4.0 | 8160 | ~1.34M |

## Common Issues

- ROOT needs `local_deps/lib/x86_64-linux-gnu` in LD_LIBRARY_PATH for libpcre
- WEXPO 4.5 gives ~94% acceptance for pp vacuum (1000 requested → ~940 good events) and ~67% for pPb with medium
- The snap-installed ROOT (`/snap/root-framework/`) conflicts with the local install — always set PATH/LD_LIBRARY_PATH explicitly
- HepMC conversion from 2000 files uses shell glob expansion (~140 KB args, within ARG_MAX)
