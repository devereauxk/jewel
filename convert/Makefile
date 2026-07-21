ROOT_CONFIG := /raid5/root/root-v6.34.04/root/bin/root-config

CXX := g++
CXXFLAGS := -O2 -Wall $(shell $(ROOT_CONFIG) --cflags)
LDFLAGS := $(shell $(ROOT_CONFIG) --libs)

all: ConvertHepMCToRoot

ConvertHepMCToRoot: ConvertHepMCToRoot.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f ConvertHepMCToRoot

.PHONY: all clean
