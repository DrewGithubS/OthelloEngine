Exec = perft
OPTS = -O3 -g

objects := $(patsubst %.c,%.o,$(wildcard *.c))

.PHONY: all
all: $(Exec)

.PHONY: check
check:
	echo Objects are $(objects)

$(objects): %.o: %.c *.h
	$(CXX) -c $(OPTS) $< -o $@
	

$(Exec): $(objects) $(cu_objects)
	$(CXX) $(OPTS) $(objects) -o $(Exec)

.PHONY: clean
clean:
	-rm *.o $(Exec)