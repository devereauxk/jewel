# Investigation: pPb Z-hadron broadening vs pp in JEWEL 2.4.0-2D-MOD

**Date:** 2026-08-03
**Samples:** `jewel_pp8160v3_MOD_500k.root` (pp), `jewel_pPb_v2_MOD_500k.root` (pPb, recoils on), `jewel_pPb_v3_MOD_norecoil_500k.root` (pPb, recoils off)
**Scope:** Research only — no code was changed. Audited `medium-2D.f` and `jewel-2.4.0.f` in `jewel-2.4.0-2D-MOD/`, surveyed all 100 hydro profile directories and all 200 production logs, and reviewed the relevant literature.

---

## Executive summary

The large pPb-vs-pp broadening of the Z-hadron Δη/Δφ correlation is **not caused by a single fatal bug**, and it is **not caused by our MOD changes**. It is dominated by JEWEL's honest response to hydro profiles that are far hotter than the "dilute pPb medium" we assumed (peak T = 280–385 MeV — central-PbPb-core-like hot spots), amplified by:

1. a **real bug in the hard-vertex sampling** (`PICKVTX`) that pins up to ~27% of events per hydro profile at the exact fireball center,
2. a **boost-invariant medium** assumption that lets every parton at any rapidity (|η| < 3.1) traverse the full droplet, directly inflating the Δη side of the correlation,
3. JEWEL's **documented tendency to overquench small systems** (its own authors report it overpredicts suppression even in O-O).

The **equal sampling of the 100 hydro directories is wrong in two independent ways** (duplicated profiles, and missing Ncoll weighting for hard probes), but correcting it would *slightly increase* the medium effect — it does not explain the excess.

Experimentally, broadening of this magnitude in min-bias pPb is **excluded by more than an order of magnitude** (CMS dijets at 8.16 TeV bound energy loss at ≤ 1.26% of jet pT even in high-multiplicity events). The pPb curves should be treated as a JEWEL model upper bound, not a prediction.

---

## 1. The hydro ensemble is not "100 Ncoll bins"

The directories under `hydro/sample/` are **not Ncoll-ordered bins**. Each is one complete minimum-bias Trajectum event, named `<jobID>.<eventNumber>` (the dot-prefixed names like `.10` have an empty job ID). Provenance: `hydro/extractTrajectumForWilke.sh` and the Trajectum config `hydro/0smllnooverjewelMB.par` (pPb 8160 GeV, `numevents=100`, freeze-out T = 149.679 MeV, free-streaming to τ₀ = 0.606 fm/c, 96×96 grid over 12×12 fm, σ_NN = 71.93 mb, EPPS21 nPDF, `entropyacceptanceprobability=0.05`).

Key facts (md5 comparison of profiles + scan of all Tcontour/NCollHisto files):

| Property | Value |
|---|---|
| Directories | 100 |
| **Unique hydro events** | **57** (11 appear 3×, 21 appear 2×, 25 once) |
| Ncoll per event | 1–21, mean 10.8, median 11 |
| Peak T (first slice, τ = 0.616 fm/c) | 174–385 MeV, median **316 MeV** |
| Profile end time | 0.92–4.15 fm/c, median 3.24 (ends at freeze-out) |
| Time slices per event | 4–36 (not uniformly 29) |
| Transverse hot region | within ~1.6 fm of center |
| Flow velocity | up to β ≈ 0.99 at edges (β ≤ 0.78 above Tc) |

Directory-name ordering carries **zero** Ncoll information (r ≈ 0.03 between lexicographic index and Ncoll). Duplicate directories produce log statistics identical to 3 decimals, confirming byte-identical profiles.

Comparison point: the JEWEL+Trajectum O-O study by Kolbe, Le Roux, Zapp (arXiv:2510.17570) quotes initial T ≈ 260–400 MeV — our pPb profiles' peaks (280–385 MeV) are in the same range, so the temperatures themselves are plausible Trajectum output, not a corrupted file or unit error. These are simply *hot little fireballs*.

**Open provenance question:** `entropyacceptanceprobability=0.05` in the Trajectum config may bias the 100-event sample toward high-entropy (high-multiplicity) events. Whether these 100 events are a *fair* MB sample should be confirmed with the person who produced them before any quantitative weighting is finalized.

## 2. Measured medium activity (production logs, 100 profiles × 5000 events each)

| Quantity | recoils on (v2) | recoils off (v3) |
|---|---|---|
| Mean scatterings/event | **10.70** (min 0.001, median 11.2, max 15.9) | **10.70** (max 15.6) |
| Mean effective scatterings | 10.22 | 10.25 |
| Mean splittings/event | 1.81 | 1.80 |
| Good events / discarded | 5000 / 0 in every log | 5000 / 0 in every log |

- Scattering count correlates strongly with the event's Ncoll (r = 0.88) and with its peak temperature (r = 0.93); v2 and v3 track each other with r = 0.985 (as expected — the recoil setting does not change the scattering rate; see §4).
- Only the Ncoll ≤ 2 events are dramatically quiet (one profile, `2.89`, is essentially medium-free: 0.001 scatterings, dead by τ = 0.92 fm/c). From Ncoll ≈ 4 to Ncoll = 21 the activity spread is only ~2×.
- The "~19 scatterings/event" quoted earlier in this project came from `logs/out_ZJet_pPb_v2_mod_test.log` — a 1000-event test on a single hot profile (`.10`) — not from production. The production mean is 10.7.
- Internal consistency: ~10.7 scatterings (summed over **all** shower partons — the counter `NSCAT` at `jewel-2.4.0.f:4710` increments per scattering-centre encounter for every active cascade parton) over a ≲ 2.6 fm/c in-medium traversal implies a mean free path of ~0.25 fm for the active partons — the expected order for an ideal-gas parton density (`n = 5.106·T³`, `medium-2D.f:649`) at T ≈ 300 MeV. The number is *self-consistent JEWEL physics on these profiles*, not a bookkeeping error.
- The earlier project note "medium too dilute, 0 scatterings across all Ncoll bins" was an artifact of the pre-MOD `MSTP(125)=2` bug (no partons were ever medium-activated). The medium was never actually sampled before the fix.

## 3. Confirmed bug: hard-vertex sampling (`PICKVTX`, medium-2D.f:391–434)

`READNCOLL` (medium-2D.f:1618–1784) correctly builds **two** arrays from `NCollHisto.dat`:

- `ncollNZ(1..count, 1:3)` — one row per *nonzero cell* (x, y, ncoll); `count` = number of nonzero cells,
- `ncollpDist(1..ncollSum, 1:2)` — one row per *binary collision* (lines 1749–1771), i.e. the correct Ncoll-weighted sampling list; `ncollSum` = Σ ncoll = event Ncoll.

`PICKVTX` then samples (lines 421–429):

```fortran
rndIndex = int(pyr(0) * ncollSum)          ! range 0 .. ncollSum-1
...
X = ncollNZ(rndIndex,1)+ (pyr(0)-0.5) * dx
Y = ncollNZ(rndIndex,2)+ (pyr(0)-0.5) * dy
```

The comment above it (lines 416–419) says the intent was to sample the duplicated per-collision list — but the code indexes **`ncollNZ`**, not `ncollpDist`. Consequences:

1. **Index 0 is an out-of-bounds read** (probability 1/ncollSum per event; for single-collision events, `int(pyr·1)=0` *always* — 100% OOB). Undefined behavior; in practice returns near-zero coordinates.
2. **When ncollSum > count** (any cell with ≥ 2 collisions), indices count+1 … ncollSum−1 hit unfilled, zero-initialized rows → **vertex pinned at (0,0) ± 0.1 fm — the fireball center / hottest point**. Fraction of pinned+garbage events per profile = (1 + max(ncollSum−1−count, 0))/ncollSum: e.g. profile `2.62` (count 11, Ncoll 15) → 4/15 ≈ **27%**; typical profiles ~5–10%.
3. When ncollSum == count, the last nonzero cell is never sampled.
4. Even for valid indices, sampling is uniform over *cells* rather than ∝ ncoll per cell (minor here — most cells hold 1 collision).

`ncollpDist` — the correct implementation — is dead code. **Net effect: a systematic excess of production vertices at the hottest point of the medium → longer in-medium paths, more scatterings, more broadening.** The correct sampling is `rndIndex = int(pyr(0)*ncollSum) + 1` indexing `ncollpDist`. (Not applied — research only.)

Also confirmed: `BMIN/BMAX/CENTRMIN/CENTRMAX` in the medium params file are sampled in `MEDNEXTEVT` (370–374) but **never used** in NCOLLHISTO mode — our `BMAX 4.93` is inert.

## 4. Verified NOT bugs (things we checked that are correct or stock-consistent)

- **The MOD changes do not inflate medium activity.** The medium cascade (`MAKECASCADE`, jewel-2.4.0.f:2319–2359) evolves only status K=1/2 partons. The MOD's simplified activation loop (DO 283, verbatim 2.2.0 logic) sets *everything* except the matrix-element jet parton to K=4 (inert). Beam remnants, spectators, ISR partons, and the Z decay muons **cannot scatter**. Activation is *narrower* than the original DO 183 loop.
- **KEEPRECOILS F changes nothing about the jet parton's kinematics.** The scattering, momentum transfer, and deflection are computed identically in both settings (`ALLHAD` is first consulted at jewel-2.4.0.f:4598, *after* all kinematics); recoils-off just marks the recoil K=13 so it is deleted before output. This is why the broadening survives in all three test modes — the elastic kicks and the induced radiation they seed are always present. Recoils-off additionally **violates 4-momentum conservation by design** (energy flows to unrecorded recoils), softening the away side.
- **Units are consistent with stock JEWEL** (T in GeV from file headers, positions in fm; the `rate = 5·n_eff·σ` construction in `GETDELTAT` is inherited unchanged from validated 2.2.0). No fm↔GeV conversion bug.
- **Freeze-out handled correctly**: T < TC = 0.17 GeV returns 0 (medium-2D.f:799); medium switches off after the last time slice (line 715) — no "last slice forever" error. Note TC = 170 MeV > Trajectum freeze-out 150 MeV, so JEWEL already ignores the coolest part of the evolution.
- **Trilinear interpolation** of T(τ,x,y) and flow u(τ,x,y) is implemented correctly; scattering centres are correctly boosted by local flow and spacetime rapidity; the flux factor γ(1−β·cosθ) is the right form.
- **Minor issues found (all benign or sub-percent):** slice time parsed from filename with `F5.3` truncates the last digit (τ₀ = 0.616 → 0.61); missing *lower*-edge grid bounds checks in GETTEMP/GETUX (extrapolated T stays below TC → clipped); possible 0/0 NaN in GETNEFF when interpolated flow is exactly zero (silently becomes "no scattering" — an *underestimate* at the fireball edge); one-sided η cut in GETNEFF masked by GETTEMP's two-sided cut; `AVSCATCEN` directs the average scatterer momentum radially instead of along local flow (affects only a cross-section estimate); MEDINIT shells out `ls` and writes `Tlist.dat`/`Vlist.dat` into the hydro directories at runtime.

## 5. Why pPb broadens so much (mechanism synthesis)

With the shower fix in place, each PPZJ event's jet parton (and its shower daughters) traverses a droplet with T > TC over fm-scale paths for ~1–3 fm/c. The medium effect stacks four contributions:

1. **Elastic kT kicks:** ~10.7 scatterings/event, each transferring kT ~ m_D (0.45–0.9 GeV, regulated t-channel exchange). A random walk of N kicks accumulates ⟨kT²⟩ ∝ N — order 1.5–3 GeV of accumulated transverse momentum spread across the cascade. Present in **all** modes (see §4).
2. **Medium-induced radiation:** the accumulated coherent Q² seeds extra emissions (JEWEL's LPM algorithm accepts a coherent set of scatterings as one emission with probability 1/N). Splittings rise from ~1.8 (pp vacuum at these settings) with additional medium-induced contributions. More, softer, wider-angle particles.
3. **Pre-hydro ramp:** GETTEMP ramps the first slice linearly from τ=0 (medium-2D.f:741–760, `T = T_slice1·τ/τ₀`), so for hot profiles the medium is already above TC from τ ≈ 0.3 fm/c — the earliest, densest phase, and a large fraction of the total path in a ~1.5 fm system. This convention is inherited from stock `medium-simple.f`, but its *relative* importance is much larger in a small, short-lived system.
4. **Longitudinal (Δη) inflation — see §6.**

On top of these, recoils-on adds the soft thermal recoils colour-connected into the jet's strings; the hole-subtraction schemes modulate how much of that survives:

- The flat "0.67 hole factor" is **not a validated scheme** — the JEWEL recoil papers (arXiv:1707.01539, arXiv:2207.14814) state that track-level observables with particle cuts (exactly our Z-hadron correlation: pT > 0.5, |η| < 2.4, charged) require **constituent-level subtraction**; 4MomSub is exact only for jet-level observables. The 0.67 rationale (partonic holes vs hadronic tracks, from the FHead analysis code) is a rough patch, not a derivation.
- Whichever hole scheme is used, it only modulates the soft component — the parton-level broadening (items 1–3) is common to all three test modes, which is exactly what we observe.

## 6. The boost-invariant medium inflates Δη specifically

The 2D profile is extended **boost-invariantly**: T and u depend only on (τ, x, y), and any parton with spacetime rapidity |η| < ETAMAX = 3.1 sees the full droplet (medium-2D.f:707–708). Two problems for pPb:

- A real pPb fireball has a *limited longitudinal extent* and rapidity-dependent density; assuming invariance over |η| < 3.1 means forward/backward shower partons scatter as if at midrapidity — directly broadening the Δη distribution of soft produced particles across our full |η| < 2.4 acceptance.
- **No CM-frame shift is applied**: pPb at 8.16 TeV has y_CM ≈ 0.465 (Pb-going direction), and the Trajectum medium is at rest in the *nucleon-nucleon* frame of its own simulation, while our JEWEL run generates symmetric pp kinematics at 8160 GeV. The asymmetry of the real system (medium pulled toward the Pb side) is absent.

Both are modeling assumptions inherited from the 2D implementation's PbPb-oriented design, not coding errors — but for a pPb Δη observable they act as pure inflation.

## 7. Is equal sampling of the hydro directories correct? No — in two ways

**(a) Duplicates.** Equal events per *directory* gives each unique hydro event a weight of 1, 2, or 3 depending on how many times it happens to be duplicated in the sample — an arbitrary, physics-free weighting affecting 32 of 57 unique events.

**(b) Missing Ncoll weighting.** For a *hard-probe* (Z+jet) sample, the probability that a given MB event hosts the hard scattering scales with its Ncoll. The hydro ensemble underlying Z+jet events must therefore be weighted **∝ Ncoll_event** relative to a fair MB sample. Correct prescriptions (either):

- generate events per unique profile ∝ its Ncoll (e.g. Ncoll=21 profile gets 21× the events of an Ncoll=1 profile), or
- keep equal statistics per unique profile and apply an analysis weight w = Ncoll/⟨Ncoll⟩ per event.

**Direction of the bias:** scattering activity rises with Ncoll (r = 0.88), so proper Ncoll weighting shifts weight toward *hotter* events and would **increase** the average medium effect by roughly 10–25% (estimate from the per-profile activity table). The current equal sampling therefore *understates* the (already too large) JEWEL medium effect — the weighting error does **not** explain the observed broadening, but it must be fixed before quoting quantitative pPb/pp ratios.

Additionally, the within-event vertex distribution should be ∝ local Ncoll density — which is exactly what the dead `ncollpDist` array implements (§3).

Caveat: both prescriptions assume the 57 unique events are a fair MB sample; verify the `entropyacceptanceprobability=0.05` selection with the sample's producer (§1).

## 8. Physics expectation: the observed effect size is excluded by experiment

- **CMS dijet pT balance, pPb 8.16 TeV** (arXiv:2504.08507): medium-induced energy loss of the subleading jet ≤ **1.26% of its pT at 90% CL**, in *high-multiplicity* events — a stronger medium than our min-bias ensemble.
- **ALICE hadron+jet, pPb 5.02 TeV** (arXiv:1712.05603): out-of-cone (R = 0.4) energy transport < **0.4 GeV/c at 90% CL**; no significant acoplanarity broadening — a direct limit on angular redistribution.
- **ATLAS jet-correlated hadrons, pPb 5.02 TeV** (arXiv:2206.01138): I_pPb consistent with unity within a few percent at all centralities.
- **Theory** (Huss et al., arXiv:2007.13754/13758): the pPb medium interaction scale is ~10× weaker than PbPb (ω̄ ≈ 0.7 vs 6.6 GeV); predicted R_pPb ≈ 1.00–1.05.
- **JEWEL's own authors** (arXiv:2510.17570, JEWEL+Trajectum O-O): standard incoherent JEWEL *overpredicts* suppression even in O-O — a hotter, larger system than pPb — and colour-coherence corrections (suppressing early-time scatterings) are needed to match ATLAS O-O data. Overprediction in pPb is expected model behavior.
- Even in **central PbPb**, the CMS Z-hadron correlation (arXiv:2507.09307) shows significant Δφ/Δη modification only for soft hadrons (1 < pT < 2 GeV) — not a gross broadening of the full correlation.

Conclusion: a pPb/pp broadening "~4× the pp-data-level differences" is not a plausible physical prediction; it is JEWEL-on-hot-Trajectum-profiles behavior, amplified by the §3 vertex bug and §6 longitudinal assumptions.

## 9. Recommended next steps (none applied — this document is research only)

1. **Fix `PICKVTX`** to sample `ncollpDist(int(pyr(0)*ncollSum)+1, :)` — removes the center-pinning and OOB indexing, and gives the intended Ncoll-density vertex distribution. Cheap, unambiguous, and should measurably reduce the medium effect.
2. **Deduplicate the hydro ensemble** (57 unique events) and **weight events ∝ Ncoll** (generation-side or analysis-side). Verify the MB-fairness of the Trajectum sample with its producer first.
3. **Quantify the pre-τ₀ ramp**: test with the ramp disabled (T = 0 for τ < 0.616 fm/c) to measure how much broadening comes from the pre-hydro phase. In a droplet this small it is likely a substantial fraction.
4. **Consider the longitudinal assumptions** (§6): restrict the medium's rapidity coverage (ETAMAX for the medium, not the ME) and/or apply the y_CM ≈ 0.465 shift. This most directly targets the Δη inflation.
5. **Analysis side:** follow arXiv:2207.14814 — constituent-style subtraction for track-level correlations; keep the recoils-off sample as the "radiative-only" baseline; treat the 0.67 hole factor as an uncontrolled approximation.
6. **Interpretation:** present JEWEL pPb curves as a model *upper bound* with the above caveats; experiment bounds the true effect at the percent level.

---

## Appendix: key file/line references

| Item | Location |
|---|---|
| Vertex sampling bug | `medium-2D.f:421,428-429` (dead correct code: 1749–1771) |
| Pre-hydro linear ramp | `medium-2D.f:741-760` |
| Medium off after last slice | `medium-2D.f:715` |
| TC freeze-out cut | `medium-2D.f:799` |
| Ideal-gas density n = 5.106·T³ | `medium-2D.f:649-650` |
| Debye mass (0.9·3T, floor 0.45) | `medium-2D.f:611-618` |
| Boost-invariant rapidity window | `medium-2D.f:707-708` |
| Unused BMIN/BMAX/centrality | `medium-2D.f:370-374` |
| Cascade activation (K=1/2 only) | `jewel-2.4.0.f:2319-2359` |
| MOD activation loop (DO 283) | `jewel-2.4.0.f:1484-1489` |
| KEEPRECOILS first consulted | `jewel-2.4.0.f:4598` |
| Scattering counters | `jewel-2.4.0.f:4710` (NSCAT), `:4869` (NSCATEFF) |
| Scattering rate sampling | `jewel-2.4.0.f:6838-6956` (GETDELTAT) |
| Trajectum provenance | `hydro/extractTrajectumForWilke.sh`, `hydro/0smllnooverjewelMB.par` |
| Production logs | `jewel-2.4.0-2D-MOD/logs/out_ZJet_pPb_{v2,v3}_*.log` (working install) |
