CXX = g++
CFLAGS = -std=c++11 -Wall -O3 -msse2  -fopenmp  -I..

BIN = ./bin/remine_train ./bin/remine_segment ./bin/genSepath ./bin/tuple_generation_train ./bin/remine
#./bin/remine_baseline ./bin/remine_rm_train
.PHONY: clean all

all: ./bin $(BIN)

./bin/remine_train: ./src/main.cpp ./src/utils/*.h ./src/frequent_pattern_mining/*.h ./src/data/*.h ./src/classification/*.h ./src/model_training/*.h
./bin/remine_segment: ./src/segment.cpp ./src/utils/*.h ./src/frequent_pattern_mining/*.h ./src/data/*.h ./src/classification/*.h ./src/model_training/*.h 
./bin/genSepath: ./src/genSepath.cpp ./src/utils/*.h
./bin/tuple_generation_train: ./src/tuple_generation_train.cpp ./src/utils/*.h ./src/frequent_pattern_mining/*.h ./src/data/*.h ./src/classification/*.h ./src/model_training/*.h ./src/genSepath.h
./bin/remine: ./src/remine_server.cpp ./src/utils/*.h ./src/frequent_pattern_mining/*.h ./src/data/*.h ./src/classification/*.h ./src/model_training/*.h ./src/genSepath.h

./bin:
	mkdir -p bin

LDFLAGS= -pthread -lm -Wno-unused-result -Wno-sign-compare -Wno-unused-variable -Wno-parentheses -Wno-format
$(BIN) :
	$(CXX) $(CFLAGS) $(LDFLAGS) -o $@ $(filter %.cpp %.o %.c, $^)
$(OBJ) :
	$(CXX) -c $(CFLAGS) -o $@ $(firstword $(filter %.cpp %.c, $^) )

clean :
	rm -rf bin