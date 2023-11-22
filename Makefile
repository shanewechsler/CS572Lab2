
# CXX Make variable for compiler
CXX=g++
# -std=c++11  C/C++ variant to use, e.g. C++ 2011
# -Wall       show the necessary warning files
# -g3         include information for symbolic debugger e.g. gdb 
CXXFLAGS=-std=c++11 -Wall -g3 -c

all: pipeSim

pipeSim : pipeSim.o Simulator.o
	$(CXX) -o pipeSim pipeSim.o Simulator.o

Simulator.o : Simulator.h Simulator.cpp
	$(CXX) $(CXXFLAGS) Simulator.cpp

pipeSim.o : Simulator.h pipeSim.cpp
	$(CXX) $(CXXFLAGS) pipeSim.cpp