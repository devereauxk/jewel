# WEXPO Parameter Study

Goal: find the WEXPO value that gives roughly equal raw event counts in the Z pT bins [0, 30) and [30, 500] GeV.

## Method

WEXPO controls importance sampling in PYTHIA via `pT_hat^WEXPO`. Higher values push more raw events to high pT-hat; EventWeight compensates so physics is unchanged, but statistical precision shifts.

Each test ran 5000 pp vacuum events at 5020 GeV using `jewel-2.4.0-vac`. The Z boson was reconstructed from the muon pair (PDG +-13, status 1) with cuts matching the converter: muon pT > 20 GeV, |eta| < 2.4, Z mass in [60, 120] GeV, |y_Z| < 2.4.

## Scripts

All in `jewel-2.4.0-2D/` (working install).

- `study_wexpo.sh` — coarse scan over WEXPO = 0, 1, 2, 3, 4 (PTMIN=5)
- `study_wexpo_fine.sh` — fine scan over WEXPO = 0.8, 1.0, 1.2, 1.4, 1.6 (PTMIN=5)
- `count_zpt.py` — parses HepMC files and counts Z candidates in each pT bin

Usage:
```bash
cd jewel-2.4.0-2D/
./study_wexpo.sh        # runs 5 JEWEL jobs in parallel, prints table
./study_wexpo_fine.sh   # same, finer grid

# standalone counting on any HepMC file(s):
python3 count_zpt.py eventfiles/out_ZJet_pp_wexpo_1.hepmc
# prints: N_total  N(0-30)  N(30-500)  ratio
```

## Results with PTMIN=5

### Coarse scan

| WEXPO | N_Z  | N(0-30) | N(30-500) | ratio |
|-------|------|---------|-----------|-------|
| 0     | 886  | 764     | 122       | 6.262 |
| 1     | 1239 | 694     | 545       | 1.273 |
| 2     | 1877 | 480     | 1397      | 0.344 |
| 3     | 2425 | 167     | 2258      | 0.074 |
| 4     | 2864 | 53      | 2811      | 0.019 |

### Fine scan

| WEXPO | N_Z  | N(0-30) | N(30-500) | ratio |
|-------|------|---------|-----------|-------|
| 0.8   | 1140 | 750     | 390       | 1.923 |
| 1.0   | 1239 | 694     | 545       | 1.273 |
| 1.2   | 1342 | 692     | 650       | 1.065 |
| 1.4   | 1452 | 636     | 816       | 0.779 |
| 1.6   | 1543 | 550     | 993       | 0.554 |

Best at PTMIN=5: **WEXPO = 1.2** (ratio 1.06).

## Results with PTMIN=0

PTMIN=0 does not diverge for Z+jet (the Z mass regulates the cross section). The PPZJ process ran without issues.

### Coarse scan

| WEXPO | N_Z  | N(0-30) | N(30-500) | ratio |
|-------|------|---------|-----------|-------|
| 0     | 761  | 684     | 77        | 8.883 |
| 0.5   | 938  | 732     | 206       | 3.553 |
| 1.0   | 1185 | 719     | 466       | 1.543 |
| 1.2   | 1285 | 705     | 580       | 1.216 |
| 1.5   | 1443 | 584     | 859       | 0.680 |
| 2.0   | 1777 | 470     | 1307      | 0.360 |

### Fine scan

| WEXPO | N_Z  | N(0-30) | N(30-500) | ratio |
|-------|------|---------|-----------|-------|
| 1.3   | 1318 | 699     | 619       | 1.129 |
| 1.35  | 1353 | 691     | 662       | 1.044 |
| 1.4   | 1408 | 699     | 709       | 0.986 |
| 1.45  | 1460 | 626     | 834       | 0.751 |

## Recommendation

**WEXPO = 1.4** with PTMIN = 0, PTMAX = 1200. This gives a low/high ratio of 0.99, nearly perfect balance between the 0-30 and 30-500 Z pT bins.
