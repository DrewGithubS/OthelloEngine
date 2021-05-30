Exec = perft
OPTS = -O3 -march=native -std=c++11
NVCCOPTS = -O3 -std=c++11

objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
cu_objects := $(patsubst %.cu,%.o,$(wildcard *.cu))

NVCC = nvcc

.PHONY: all
all: $(Exec)

.PHONY: check
check:
	echo Objects are $(objects)

$(objects): %.o: %.cpp *.h
	$(CXX) $(OPTS) -c $< -o $@

$(cu_objects): %.o: %.cu *.cudah
	$(NVCC) $(NVCCOPTS) -c $< -o $@
	#$(CXX) $< $(NVCCOPTS) $@
	

$(Exec): $(objects) $(cu_objects)
	$(CXX) $(OPTS) -pthread $(objects) $(cu_objects) -o $(Exec)

.PHONY: clean
clean:
	-rm *.o $(Exec)