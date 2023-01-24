CFLAGS=-std=c17 -Wall -O3

objects=coords.o state.o stateio.o stateutil.o


test: $(objects)

run-test: test
	@./test


coords.o: coords.h state.h
state.o: coords.h state.h
stateio.o: coords.h errorcodes.h state.h stateio.h stateutil.h
stateutil.o: state.h
test.o: state.h stateio.h stateutil.h


clean:
	rm -f $(objects)
	rm -f test


.PHONY: clean

# vim: set noexpandtab sts=0:
