# JEWEL with 2D hydro profile

Kyle Devereaux

Forked from Isobel Kolbe's jewel-2.4.0-2D codebase https://github.com/isobelkolbe/jewel-2.4.0-2D/

JEWEL manual https://arxiv.org/pdf/1311.0048

## Directory structure

- `jewel-2.4.0-2D/` Main workspace: vacuum, simple, and 2D hydro medium executables
- `jewel-2.2.0/` Older JEWEL version (vacuum only, for cross-version validation)
- `convert/` HepMC-to-ROOT conversion and validation plotting tools
- `hydro/sample/` 100 Ncoll bins of Trajectum hydro profiles (8.16 TeV pPb)
- `lhapdf/` LHAPDF 6.5.5 (lib/ and share/LHAPDF/)
- `local_deps/` System libraries needed at runtime (libpcre, etc.)

## Conversion and validation

The `convert/` directory contains the HepMC-to-ROOT converter and validation tools, built with ROOT 6.34.04. See `convert/` for source and usage details.

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
