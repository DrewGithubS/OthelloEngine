Exec = perft
OPTS = -O3 -std=c++11 -g

objects := $(patsubst %.cpp,%.o,$(wildcard *.cpp))

.PHONY: all
all: $(Exec)

.PHONY: check
check:
	echo Objects are $(objects)

$(objects): %.o: %.cpp *.h
	$(CXX) -c $(OPTS) $< -o $@
	

$(Exec): $(objects) $(cu_objects)
	$(CXX) $(OPTS) $(objects) -o $(Exec)

.PHONY: clean
clean:
	-rm *.o $(Exec)