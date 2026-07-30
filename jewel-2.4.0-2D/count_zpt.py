#!/usr/bin/env python3
"""Count reconstructed Z pT in bins from JEWEL HepMC output."""
import sys
import math

def count_zpt(filename):
    mu_plus = []   # (px, py, pz, E) for PDG -13
    mu_minus = []  # (px, py, pz, E) for PDG +13
    n_low = 0      # Z pT in [0, 30)
    n_high = 0     # Z pT in [30, 500]
    n_total = 0

    with open(filename) as f:
        for line in f:
            if line.startswith("E "):
                if mu_plus and mu_minus:
                    for mp in mu_plus:
                        for mm in mu_minus:
                            px = mp[0] + mm[0]
                            py = mp[1] + mm[1]
                            pz = mp[2] + mm[2]
                            E = mp[3] + mm[3]
                            mass = math.sqrt(max(0, E*E - px*px - py*py - pz*pz))
                            if mass < 60 or mass > 120:
                                continue
                            pt = math.sqrt(px*px + py*py)
                            if E > abs(pz):
                                y = 0.5 * math.log((E + pz) / (E - pz))
                            else:
                                continue
                            if abs(y) > 2.4:
                                continue
                            n_total += 1
                            if pt < 30:
                                n_low += 1
                            else:
                                n_high += 1
                mu_plus = []
                mu_minus = []

            elif line.startswith("P "):
                parts = line.split()
                pdg = int(parts[2])
                status = int(parts[8])
                if status != 1:
                    continue
                if abs(pdg) != 13:
                    continue
                px = float(parts[3])
                py = float(parts[4])
                pz = float(parts[5])
                E = float(parts[6])
                pt_mu = math.sqrt(px*px + py*py)
                if pt_mu < 20:
                    continue
                p_tot = math.sqrt(px*px + py*py + pz*pz)
                if p_tot > abs(pz):
                    eta = 0.5 * math.log((p_tot + pz) / (p_tot - pz))
                else:
                    continue
                if abs(eta) > 2.4:
                    continue
                if pdg == 13:
                    mu_minus.append((px, py, pz, E))
                else:
                    mu_plus.append((px, py, pz, E))

    # flush last event
    if mu_plus and mu_minus:
        for mp in mu_plus:
            for mm in mu_minus:
                px = mp[0] + mm[0]
                py = mp[1] + mm[1]
                pz = mp[2] + mm[2]
                E = mp[3] + mm[3]
                mass = math.sqrt(max(0, E*E - px*px - py*py - pz*pz))
                if mass < 60 or mass > 120:
                    continue
                pt = math.sqrt(px*px + py*py)
                if E > abs(pz):
                    y = 0.5 * math.log((E + pz) / (E - pz))
                else:
                    continue
                if abs(y) > 2.4:
                    continue
                n_total += 1
                if pt < 30:
                    n_low += 1
                else:
                    n_high += 1

    return n_total, n_low, n_high

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} file1.hepmc [file2.hepmc ...]")
        sys.exit(1)

    total, low, high = 0, 0, 0
    for fn in sys.argv[1:]:
        t, l, h = count_zpt(fn)
        total += t
        low += l
        high += h

    ratio = low / high if high > 0 else float('inf')
    print(f"{total}\t{low}\t{high}\t{ratio:.3f}")
