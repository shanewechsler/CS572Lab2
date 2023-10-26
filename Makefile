
# CXX Make variable for compiler
CXX=g++
# -std=c++11  C/C++ variant to use, e.g. C++ 2011
# -Wall       show the necessary warning files
# -g3         include information for symbolic debugger e.g. gdb 
CXXFLAGS=-std=c++11 -Wall -g3 -c

all: gprSim

gprSim : gprSim.o Simulator.o
	$(CXX) -o gprSim gprSim.o Simulator.o

Simulator.o : Simulator.h Simulator.cpp
	$(CXX) $(CXXFLAGS) Simulator.cpp

gprSim.o : Simulator.h gprSim.cpp
	$(CXX) $(CXXFLAGS) gprSim.cpp