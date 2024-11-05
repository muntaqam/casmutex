CFLAGS := -Wall -Werror -g -std=c99 -D_DEFAULT_SOURCE -pthread
LDFLAGS := -pthread
CC := gcc

TESTS := test_simple-lock test_counters

all: $(TESTS)

test: $(TESTS)
	@for test in $(TESTS); do          \
	    echo -n "testing $$test ... "; \
	    ./$$test;                      \
	done; true

clean:
	rm -f *~ src/*~ src/*.o tests/*~ tests/*.o $(TESTS)

test_%: tests/%.o src/csemutex.o
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: all clean
