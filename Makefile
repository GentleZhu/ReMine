CXX = g++
CFLAGS = -std=c++11 -Wall -O3 -msse2  -fopenmp  -I..

BIN = ./bin/remine_train ./bin/remine_segment ./bin/remine_baseline
.PHONY: clean all

all: ./bin $(BIN)

./bin/remine_train: ./src/main.cpp ./src/utils/*.h ./src/frequent_pattern_mining/*.h ./src/data/*.h ./src/classification/*.h ./src/model_training/*.h ./src/clustering/*.h
./bin/remine_segment: ./src/segment.cpp ./src/utils/*.h ./src/frequent_pattern_mining/*.h ./src/data/*.h ./src/classification/*.h ./src/model_training/*.h ./src/clustering/*.h
./bin/remine_baseline: ./src/remine_baseline.cpp ./src/utils/*.h

./bin:
	mkdir -p bin

LDFLAGS= -pthread -lm -Wno-unused-result -Wno-sign-compare -Wno-unused-variable -Wno-parentheses -Wno-format
$(BIN) :
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.cpp %.o %.c, $^)
$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c, $^) )

clean :
	rm -rf bin
