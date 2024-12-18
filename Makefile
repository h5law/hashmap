CC = clang
CFLAGS = -xc -std=c11
LDFLAGS = -I./src/ -I./deps/ -L./deps/
LDLIBS = -lxxhash

LIBSRC = map.c
TESTS = map_test vector_test

# $(CC) $(CFLAGS) $(LDFLAGS) ./tests/vector_test.c -o ./tests/vector_test
lib:
	$(CC) $(CFLAGS) -I./src/ -I./deps/ -c ./src/$(LIBSRC) -o ./inc/map.o
	ar rcs ./inc/libmap.a ./inc/map.o
	ranlib ./inc/libmap.a
	cp ./src/*.h ./src/*.c ./inc/

all:
	echo "all..."

test: $(TESTS)

$(TESTS):
	$(CC) $(CFLAGS) -I./inc/ -L./inc/ -I./deps/ -L./deps/ -lmap -lxxhash ./tests/$@.c -o ./tests/$@

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: all clean test
