all: jewel-2.2.0-vac

LHAPDF_PATH := /raid5/data/kdevero/jewel_workspace/lhapdf/lib

FC := gfortran
FFLAGS := -g -O2

jewel-2.2.0-vac: jewel-2.2.0.o medium-vac.o pythia6425mod.o meix.o dummies.o
	$(FC) -o $@ -L$(LHAPDF_PATH) $^ -lLHAPDF -lstdc++

clean:
	rm -f medium-*.o jewel*.o pythia6425mod.o meix.o dummies.o *~

.PHONY: all clean
