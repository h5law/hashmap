CFLAGS = -O3 -Ofast -DDEBUG_TESTS
# CFLAGS += -Wall -Wpedantic -Werror
# CFLAGS += -g -MD

all: clean test

test:
	$(CC) $(CFLAGS) -o test map.c && ./test

clean:
	rm -f test

.PHONY: all clean test
