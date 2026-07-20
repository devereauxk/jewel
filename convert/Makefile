ROOT_CONFIG := /raid5/root/root-v6.34.04/root/bin/root-config

CXX := g++
CXXFLAGS := -O2 -Wall $(shell $(ROOT_CONFIG) --cflags)
LDFLAGS := $(shell $(ROOT_CONFIG) --libs)

all: ConvertHepMCToRoot ValidateJewel ValidateJewel3Way ValidateJewelPPb

ConvertHepMCToRoot: ConvertHepMCToRoot.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

ValidateJewel: ValidateJewel.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

ValidateJewel3Way: ValidateJewel3Way.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

ValidateJewelPPb: ValidateJewelPPb.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f ConvertHepMCToRoot ValidateJewel ValidateJewel3Way ValidateJewelPPb

.PHONY: all clean
