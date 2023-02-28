CFLAGS=-std=gnu17 -Wall -O3
LDFLAGS=-lm

objects=coords.o mcts.o state.o stateio.o stateutil.o


ZOE_PORT ?= 8000


all: zoe


zoe: $(objects)


server: zoe
	uvicorn server:app --port $(ZOE_PORT) --reload


test: $(objects)


run-test: test
	@./test


bench: $(objects)


run-bench: bench
	@./bench


bench.o: bench.h coords.h
coords.o: coords.h state.h
mcts.o: mcts.h state.h
state.o: coords.h errorcodes.h state.h
stateio.o: coords.h errorcodes.h state.h stateio.h stateutil.h
stateutil.o: state.h
test.o: state.h stateio.h stateutil.h
zoe.o: errorcodes.h mcts.h state.h stateio.h


clean:
	rm -f $(objects)
	rm -f test
	rm -f bench
	rm -f zoe


.PHONY: clean run-test server

# vim: set noexpandtab sts=0:
