CFLAGS=-std=gnu17 -Wall -O3

objects=coords.o state.o stateio.o stateutil.o


ZOE_PORT ?= 8000


all: zoe


zoe: $(objects)


server: zoe
	uvicorn server:app --port $(ZOE_PORT) --reload


test: $(objects)

run-test: test
	@./test


coords.o: coords.h state.h
state.o: coords.h errorcodes.h state.h
stateio.o: coords.h errorcodes.h state.h stateio.h stateutil.h
stateutil.o: state.h
test.o: state.h stateio.h stateutil.h
zoe.o: errorcodes.h state.h stateio.h


clean:
	rm -f $(objects)
	rm -f test
	rm -f zoe


.PHONY: clean run-test server

# vim: set noexpandtab sts=0:
