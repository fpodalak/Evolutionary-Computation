CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -static
SRC = src/main.cpp src/TSPSolver.cpp src/LMSearch.cpp
HDR = src/TSPSolver.h src/Types.h src/instance.h src/construct.h

ifeq ($(OS),Windows_NT)
  EXE = .exe
  RUN = .\\
  RM = del /Q /F
else
  EXE =
  RUN = ./
  RM = rm -f
endif

tsp$(EXE): $(SRC) $(HDR)
	$(CXX) $(CXXFLAGS) -o tsp$(EXE) $(SRC)

run: tsp$(EXE)
	$(RUN)tsp$(EXE)

test: tests/test_lm.cpp src/TSPSolver.cpp src/LMSearch.cpp $(HDR)
	$(CXX) $(CXXFLAGS) -o test_lm$(EXE) tests/test_lm.cpp src/TSPSolver.cpp src/LMSearch.cpp
	$(RUN)test_lm$(EXE)

clean:
	-$(RM) tsp$(EXE) test_lm$(EXE)

.PHONY: run test clean
