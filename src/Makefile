CFLAGS=-std=c99 -Wall -O3

objects=state.o stateio.o


test: $(objects)

run-test: test
	@./test


state.o: state.h
stateio.o: state.h stateio.h


clean:
	rm -f $(objects)
	rm -f test


.PHONY: clean

# vim: set noexpandtab sts=0:
