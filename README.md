# JEWEL with 2D hydro profile

Kyle Devereaux

Forked from Isobel Kolbe's jewel-2.4.0-2D codebase https://github.com/isobelkolbe/jewel-2.4.0-2D/

JEWEL manual https://arxiv.org/pdf/1311.0048

## Directory structure

- `jewel-2.4.0-2D/` Main workspace: vacuum, simple, and 2D hydro medium executables (unmodified upstream)
- `jewel-2.4.0-2D-MOD/` Modified JEWEL 2.4.0 with bug fixes for PPZJ (see below)
- `jewel-2.2.0/` Older JEWEL version (vacuum only, for cross-version validation)
- `convert/` HepMC-to-ROOT converter (ConvertHepMCToRoot)
- `validation/` Validation/comparison tools, run scripts, plots, ROOT files
- `hydro/sample/` 100 Ncoll bins of Trajectum hydro profiles (8.16 TeV pPb)
- `lhapdf/` LHAPDF 6.5.5 (lib/ and share/LHAPDF/)
- `local_deps/` System libraries needed at runtime (libpcre, etc.)

## Produced ROOT files

All ROOT files are in `validation/`. The reference files `jewel_pp-v9.root` and `jewel_pp-v7.root` are in the workspace root.

| File | JEWEL | System | Energy | PTMIN | PTMAX | WEXPO | PDFSET | nPDF | Recoils | Events | Config |
|------|-------|--------|--------|-------|-------|-------|--------|------|---------|--------|--------|
| `jewel_pp_220.root` | 2.2.0 | pp | 5020 | 15 | 1200 | 4.5 | 10042 | off (NSET=0) | n/a | 2,000,000 | `ZJet_pp.dat` |
| `jewel_pp_220_v2_500k.root` | 2.2.0 | pp | 5020 | 0 | 1200 | 1.4 | 10042 | off (NSET=0) | n/a | 500,000 | `ZJet_pp_v2.dat` |
| `jewel_pp_240.root` | 2.4.0 | pp | 5020 | 15 | 1200 | 4.5 | 10042 | off | n/a | 1,886,306 | `ZJet_pp.dat` |
| `jewel_pp8160_2M.root` | 2.4.0 | pp | 8160 | 15 | 1200 | 4.5 | 10042 | off | n/a | 1,831,529 | `ZJet_pp_8160.dat` |
| `jewel_pp8160v2_500k.root` | 2.4.0 | pp | 8160 | 5 | 1200 | 1.2 | 10042 | off | n/a | 468,857 | `ZJet_pp8160v2.dat` |
| `jewel_pp8160v3_500k.root` | 2.4.0 | pp | 8160 | 0 | 1200 | 1.4 | 10042 | off | n/a | 468,816 | `ZJet_pp8160v3.dat` |
| `jewel_pp8160_wexpo4p5_500k.root` | 2.4.0 | pp | 8160 | 0 | 1200 | 4.5 | 10042 | off | n/a | 472,273 | `ZJet_pp8160_wexpo4p5.dat` |
| `jewel_pPb_2M.root` | 2.4.0-2D | pPb | 8160 | 15 | 1200 | 4.5 | 10042 | off | on | 1,340,059 | `ZJet_pPb.dat` |
| `jewel_pPb_v2_500k.root` | 2.4.0-2D | pPb | 8160 | 0 | 1200 | 1.4 | 10042 | off | on | 468,584 | `ZJet_pPb_v2.dat` |
| `jewel_pp8160v3_MOD_500k.root` | 2.4.0-2D-MOD | pp | 8160 | 0 (unclamped) | 1200 | 1.4 | 10042 | off | n/a | 500,000 | `ZJet_pp8160v3.dat` |
| `jewel_pPb_v2_MOD_500k.root` | 2.4.0-2D-MOD | pPb | 8160 | 0 (unclamped) | 1200 | 1.4 | 10042 | off | on | 500,000 | `ZJet_pPb_v2.dat` |
| `jewel_pPb_v3_MOD_norecoil_500k.root` | 2.4.0-2D-MOD | pPb | 8160 | 0 (unclamped) | 1200 | 1.4 | 10042 | off | off | 500,000 | `ZJet_pPb_v3.dat` |
| `jewel_pPb_v4_MOD_500k.root` | 2.4.0-2D-MOD | pPb | 8160 | 0 (unclamped) | 1200 | 1.4 | 10042 | off | on | ~500,000 | `ZJet_pPb_v4.dat` |
| `jewel_pp-v9.root` (ref) | ? | pp | 5020 | ? | ? | ? | ? | ? | ? | 2,000,000 | FHead external |
| `jewel_pp-v7.root` (ref) | ? | pp | 5020 | ? | ? | ? | ? | ? | ? | 100,000 | FHead external |

All pp vacuum runs use PROCESS PPZJ, ISOCHANNEL PP, HADRO T. pPb runs with recoils on additionally use KEEPRECOILS T, COMPRESS T, WRITESCATCEN T, WRITEDUMMIES T. The no-recoil pPb run uses KEEPRECOILS F with no hole subtraction needed.

**2.4.0-2D-MOD** fixes three bugs: (1) `MSTP(125)=2` disabled the parton shower for PPZJ events (zero splittings, zero medium scatterings), (2) PTMIN was unconditionally clamped to 3 GeV, silently overriding PTMIN=0 configs for Z/W+jet processes (where the Z mass regulates the cross section), and (3) `PICKVTX` sampled vertex positions from the wrong array (`ncollNZ` instead of `ncollpDist`), pinning 5-27% of vertices at the fireball center (0,0). Files marked "unclamped" use the true PTMIN=0 — all other PTMIN=0 entries were actually PTMIN=3. The v4 sample additionally deduplicates the 100 hydro directories (57 unique Trajectum events) and weights events proportional to Ncoll. See `jewel-2.4.0-2D-MOD/CHANGES.diff` for details. Unmodified 2.4.0/2.4.0-2D samples above have no parton shower evolution and should not be used for physics analysis.

WEXPO controls importance sampling via `pT_hat^WEXPO`. All events carry EventWeight to compensate. WEXPO=1.4 with PTMIN=0 gives roughly equal raw event counts in Z pT bins [0,30) and [30,500] GeV (see `jewel-2.4.0-2D/wexpo_study.md`).

## Conversion and validation

The `convert/` directory contains the HepMC-to-ROOT converter. The `validation/` directory contains validation/comparison tools and run scripts. Both are built with ROOT 6.34.04.

```bash
cd convert/ && make       # builds ConvertHepMCToRoot
cd validation/ && make    # builds ValidateJewel, ValidateJewel3Way, ValidateJewelPPb, CompareTrackPt
```

## JEWEL 2.4.0-2D-MOD

A modified copy of `jewel-2.4.0-2D/` with two bug fixes for Z/W+jet (PPZJ) processes. All changes are documented in `jewel-2.4.0-2D-MOD/CHANGES.diff`.

### Bug 1: MSTP(125)=2 disables parton shower for PPZJ

JEWEL 2.4.0 sets `MSTP(125)=2` to preserve ISR shower history in the PYTHIA 6 event record. This is required for the PPJJ process's special ISR handling, but breaks PPZJ: the jet parton gets an intermediate copy (K=13) that the LME finder cannot activate, so no partons enter MAKECASCADE — resulting in zero splittings and zero medium scatterings.

**Fix:** Guard `MSTP(125)=2` for PPJJ only. Use the simpler 2.2.0-style deactivation logic (DO 283) for all other processes.

### Bug 2: PTMIN unconditionally clamped to 3 GeV

Line 575 clamps `PTMIN` to a minimum of 3 GeV for all processes. This protects divergent processes like PPJJ (1/pT^4 singularity), but is unnecessary for Z/W+jet where the Z mass regulates the cross section. The clamp silently overrode all PTMIN=0 configs to PTMIN=3.

**Fix:** Make the clamp conditional — Z/W+jet processes (PPZJ, PPZQ, PPZG, PPWJ, PPWQ, PPWG) are exempt.

### Build

```bash
cd jewel-2.4.0-2D-MOD/
make            # builds jewel-2.4.0-vac, jewel-2.4.0-simple, jewel-2.4.0-2D
```

Hydro profiles are symlinked from `hydro/sample/` (same as the unmodified install).

## JEWEL 2.2.0

An older JEWEL version is installed at `jewel-2.2.0/` for cross-version validation. It builds only the vacuum executable (`jewel-2.2.0-vac`). Note: the source internally identifies as JEWEL 2.1.0 (the version banner was never updated).

## JEWEL install

These are just the commands I used to install it to my directory. Update the target directories to install it where you want.

### Install LHAPDF on Smvit

```bash
mkdir Jewel
cd Jewel
mkdir lhapdf

[copy lhapdf tar to this directory, get this from their hepforge site]
tar xf LHAPDF-6.5.5.tar.gz
cd LHAPDF-6.5.5

export PATH=$PATH:/home/kdeverea/Jewel/lhapdf
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/kdeverea/Jewel/lhapdf
export PYTHONPATH=$PYTHONPATH:/home/kdeverea/Jewel/lhapdf/lib/python3.9/site-packages

./configure prefix=/home/kdeverea/Jewel/lhapdf --disable-python
make
make install
```

### Install LHAPDF on Grendel

LHAPDF 6.5.5 is pre-installed at `lhapdf/` in the workspace root, with the `cteq6l1` PDF set (PDFSET 10042) already downloaded to `lhapdf/share/LHAPDF/`.

The JEWEL Makefile links against this local install:

```makefile
LHAPDF_PATH := /raid5/data/kdevero/jewel_workspace/lhapdf/lib
```

At runtime, `source setup.sh` (in `jewel-2.4.0-2D/`) sets:

```bash
export LD_LIBRARY_PATH=/raid5/data/kdevero/jewel_workspace/lhapdf/lib:$LD_LIBRARY_PATH
export LHAPDF_DATA_PATH=/raid5/data/kdevero/jewel_workspace/lhapdf/share/LHAPDF
```

No CVMFS or LHAPATH is used — JEWEL reads PDFs directly from the local `lhapdf/share/LHAPDF/` directory via `LHAPDF_DATA_PATH`.

### Install JEWEL

```bash
export LHAPATH=/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current

cd ..
[copy jewel tar to this directory, get this from their hepforge site]
tar xvzf jewel-2.4.0.tar.gz
cd jewel-2.4.0
[change LHAPDF_PATH := /home/kdeverea/Jewel/lhapdf/lib in Makefile]
make
```

This should compile and produce `./jewel-2.4.0-simple` and `./jewel-2.4.0-vac`. You can run these examples as a test. To run JEWEL, put the parameter file as an argument, for example

```bash
./jewel-2.4.0-simple params.example.dat
```

For ease of use, put these in your `~/.bashrc` so you don't need to call them each time.

```bash
export PATH=$PATH:/home/kdeverea/Jewel/lhapdf
export LD_LIBRARY_PATH=/home/kdeverea/Jewel/lhapdf/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH:/home/kdeverea/Jewel/lhapdf/lib/python3.9/site-packages
export LHAPATH=/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current
```

## Isobel's 2D hydro profile

https://github.com/isobelkolbe/jewel-2.4.0-2D

- Copy Isobel's `medium-2D.f` AND `jewel-2.4.0.f` to your directory. Doesn't say to copy `jewel-2.4.0.f` in Isobel's readme but you need to!
- I had to change line `160-161` of `medium-2D.f` from `character*80 FILE, buffer` to
```
character*80 FILE
character*300 buffer
```
for the medium parameter file to be read correctly.
