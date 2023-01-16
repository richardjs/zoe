CFLAGS=-std=c17 -Wall -O3

objects=state.o stateio.o stateutil.o


test: $(objects)

run-test: test
	@./test


state.o: state.h
stateio.o: errorcodes.h state.h stateio.h stateutil.h


clean:
	rm -f $(objects)
	rm -f test


.PHONY: clean

# vim: set noexpandtab sts=0:
