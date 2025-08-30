CFLAGS = -O3 -Ofast -MD -g -DTEST_IMPLEMENTATION=1

all: clean test

test:
	$(CC) $(CFLAGS) -o test map.c

clean:
	rm test

.PHONY: all clean test
