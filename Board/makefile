Exec = perft
OPTS = -O3 -march=native -std=c++11 -g
NVCCOPTS = -O3 -std=c++11 -g

objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))
cu_objects := $(patsubst %.cu,%.o,$(wildcard *.cu))

NVCC = nvcc

.PHONY: all
all: $(Exec)

.PHONY: check
check:
	echo Objects are $(objects)

$(objects): %.o: %.cpp *.h
	$(NVCC) -c $(NVCCOPTS) $< -o $@

$(cu_objects): %.o: %.cu *.cudah
	$(NVCC) -c $(NVCCOPTS) $< -o $@
	#$(CXX) $< $(NVCCOPTS) $@
	

$(Exec): $(objects) $(cu_objects)
	$(NVCC) $(NVCCOPTS) $(objects) $(cu_objects) -o $(Exec)

.PHONY: clean
clean:
	-rm *.o $(Exec)