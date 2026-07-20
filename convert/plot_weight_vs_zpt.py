import uproot
import numpy as np
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

f1 = uproot.open('jewel_pp_10k.root')
f2 = uproot.open('/raid5/data/kdevero/jewel_workspace/jewel_pp-v7.root')

ew1 = f1['Tree']['EventWeight'].array(library='np')
ew2 = f2['Tree']['EventWeight'].array(library='np')
zpt1_raw = f1['Tree']['genZPt'].array()
zpt2_raw = f2['Tree']['genZPt'].array()

zpt1 = np.array([float(z[0]) for z in zpt1_raw if len(z) > 0])
w1 = np.array([float(ew1[i]) for i, z in enumerate(zpt1_raw) if len(z) > 0])
zpt2 = np.array([float(z[0]) for z in zpt2_raw if len(z) > 0])
w2 = np.array([float(ew2[i]) for i, z in enumerate(zpt2_raw) if len(z) > 0])

fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 5.5))

xbins = np.linspace(0, 3, 51)
ybins = np.linspace(-15, -4, 51)

for ax, zpt, w, label in [(ax1, zpt1, w1, 'JEWEL 2.2.0 (this work)'),
                            (ax2, zpt2, w2, 'Reference v7')]:
    lzpt = np.log10(np.clip(zpt, 1, None))
    lw = np.log10(np.clip(w, 1e-20, None))
    h, xe, ye = np.histogram2d(lzpt, lw, bins=[xbins, ybins])
    h[h == 0] = np.nan
    pcm = ax.pcolormesh(xe, ye, h.T, cmap='viridis', norm=mcolors.LogNorm(vmin=1, vmax=h[~np.isnan(h)].max()))
    ax.set_xlabel(r'$\log_{10}(Z\, p_T$ [GeV])', fontsize=13)
    ax.set_ylabel(r'$\log_{10}$(EventWeight)', fontsize=13)
    ax.set_title(label, fontsize=14)
    fig.colorbar(pcm, ax=ax, label='Counts')

plt.tight_layout()
plt.savefig('plots/weight_vs_zpt.png', dpi=150)
print('Saved plots/weight_vs_zpt.png')
