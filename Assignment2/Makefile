CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra

all: greedy tests

greedy: src/main.cpp src/instance.h src/construct.h src/assignment1.h
	$(CXX) $(CXXFLAGS) -o $@ src/main.cpp

tests: tests/tests.cpp src/instance.h src/construct.h src/assignment1.h
	$(CXX) $(CXXFLAGS) -o tests/tests tests/tests.cpp

test: tests
	./tests/tests

run: greedy
	mkdir -p results && ./greedy

clean:
	rm -f greedy tests/tests

.PHONY: all test run clean tests
