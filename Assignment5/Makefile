CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall
LDFLAGS = -static-libstdc++ -static-libgcc
SRC = src/main.cpp src/TSPSolver.cpp

greedy: $(SRC) src/TSPSolver.h src/Types.h src/instance.h src/construct.h
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SRC)

run: greedy
	mkdir -p results && ./greedy

clean:
	rm -f greedy

.PHONY: run clean
