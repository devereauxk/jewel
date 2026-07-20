all: jewel-2.4.0-vac jewel-2.4.0-simple jewel-2.4.0-2D

# path to LHAPDF library
LHAPDF_PATH := /raid5/data/kdevero/jewel_workspace/lhapdf/lib

FC := gfortran
FFLAGS := -O2

jewel-2.4.0-vac: jewel-2.4.0.o medium-vac.o pythia6425mod-lhapdf6.o meix.o
	$(FC) -o $@ -L$(LHAPDF_PATH) $^ -lLHAPDF -lstdc++

jewel-2.4.0-simple: jewel-2.4.0.o medium-simple.o pythia6425mod-lhapdf6.o meix.o
	$(FC) -o $@ -L$(LHAPDF_PATH) $^ -lLHAPDF -lstdc++

jewel-2.4.0-2D: medium-2D.o jewel-2.4.0.o pythia6425mod-lhapdf6.o meix.o
	$(FC) -o $@ -L$(LHAPDF_PATH) $^ -lLHAPDF -lstdc++

clean:
	rm -f medium-*.o 
	rm -f jewel*.o
	rm -f pythia6425mod-lhapdf6.o meix.o
	rm -f *~

.PHONY: all
