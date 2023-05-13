CFLAGS=-std=gnu17 -Wall -O3
LDFLAGS=-lm

objects=book.o coords.o examine.o mcts.o minimax.o simulate.o state.o stateio.o stateutil.o think.o uhp.o


ZOE_PORT ?= 8000


default: zoe zoe_uhp


zoe: $(objects)
zoe_uhp: $(objects)


all: zoe zoe_uhp test bench


server: zoe
	uvicorn server:app --host 0.0.0.0 --port $(ZOE_PORT) --reload


test: $(objects)


run-test: test
	@./test


bench: $(objects)


run-bench: bench
	@./bench


debug: zoe zoe_uhp test bench
debug: CFLAGS+=-g


slow: zoe test bench
slow: CFLAGS+=-O0


watch-sims: zoe
watch-sims: CFLAGS+=-D WATCH_SIMS


bench.o: bench.h coords.h
book.o: state.h
coords.o: coords.h state.h
mcts.o: mcts.h simulate.h state.h
minimax.o: minimax.h state.h
simulate.o: simulate.h state.h
state.o: coords.h errorcodes.h state.h
stateio.o: coords.h errorcodes.h state.h stateio.h stateutil.h
stateutil.o: state.h
test.o: minimax.h state.h stateio.h stateutil.h
think.o: mcts.h state.h
uhp.o: mcts.h state.h think.h uhp.h
zoe.o: book.h errorcodes.h examine.h mcts.h minimax.h state.h stateio.h think.h uhp.h
zoe_uhp.p: think.h uhp.h


format:
	clang-format -i *.c *.h --style=WebKit


clean:
	rm -f $(objects)
	rm -f test
	rm -f bench
	rm -f zoe
	rm -f zoe_uhp


.PHONY: clean format run-test server

# vim: set noexpandtab sts=0:
