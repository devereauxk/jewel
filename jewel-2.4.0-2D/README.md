# JEWEL with 2D hydro profile

Kyle Devereaux

Forked from Isobel Kolbe's jewel-2.4.0-2D codebase https://github.com/isobelkolbe/jewel-2.4.0-2D/

Jewel manual https://arxiv.org/pdf/1311.0048


## JEWEL install

These are just the commands I used to install it to my directory. Update the target directories to install it where you want.

### Install LHAPDF

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

### Install jewel

```bash
export LHAPATH=/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current

cd ..
[copy jewel tar to this directory, get this from their hepforge site]
tar xvzf jewel-2.4.0.tar.gz
cd jewel-2.4.0
[change LHAPDF_PATH := /home/kdeverea/Jewel/lhapdf/lib in Makefile]
make
```

This should compile and produce `./jewel-2.4.0-simple`  and `./jewel-2.4.0-vac` . You can run these examples as a test. To run jewel, put the parameter file as an argument, for example

```bash
./jewel-2.4.0-simple params.example.dat
```

For ease of use, put these in your `~/.bashrc` so you don’t need to call them each time.

```bash
export PATH=$PATH:/home/kdeverea/Jewel/lhapdf
export LD_LIBRARY_PATH=/home/kdeverea/Jewel/lhapdf/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$PYTHONPATH:/home/kdeverea/Jewel/lhapdf/lib/python3.9/site-packages
export LHAPATH=/cvmfs/sft.cern.ch/lcg/external/lhapdfsets/current
```

## Isobel’s 2D hydro profile

https://github.com/isobelkolbe/jewel-2.4.0-2D

- Copy Isobel’s`medium-2D.f` AND `jewel-2.4.0.f`  to your directory. Doesn’t say to copy `jewel-2.4.0.f` in Isobel’s readme but you need too!
- I had to change line `160-161` of `medium-2D.f` from `character*80 FILE, buffer` to
```
character*80 FILE
character*300 buffer
```
for the medium parameter file to be read correctly.
