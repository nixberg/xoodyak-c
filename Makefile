COMMON = -fPIC -std=c99 -march=native -Wall -Werror -Wextra -pedantic -Wshadow 
CFLAGS = $(COMMON) -O3
TEST_CFLAGS = $(COMMON) -g3 -fsanitize=address,undefined

HEADERS = include/*.h src/*.h
OBJECTS = .build/xoodoo.o .build/xoodyak.o .build/xoodyak-internal.o


all: xoodyak.so keyed-xoodyak.so

xoodyak.so: $(OBJECTS)
	cc $(CFLAGS) -shared -o xoodyak.so $(OBJECTS)

keyed-xoodyak.so: $(OBJECTS) .build/keyed-xoodyak.o
	cc $(CFLAGS) -shared -o keyed-xoodyak.so $(OBJECTS) .build/keyed-xoodyak.o


.build/xoodoo.o: $(HEADERS) src/xoodoo_internal.c
	cc $(CFLAGS) -Iinclude -DNDEBUG -c -o .build/xoodoo.o src/xoodoo_internal.c

.build/xoodyak.o: $(HEADERS) src/xoodyak.c
	cc $(CFLAGS) -Iinclude -c -o .build/xoodyak.o src/xoodyak.c

.build/keyed-xoodyak.o: $(HEADERS) src/keyed_xoodyak.c
	cc $(CFLAGS) -Iinclude -c -o .build/keyed-xoodyak.o src/keyed_xoodyak.c

.build/xoodyak-internal.o: $(HEADERS) src/xoodyak_internal.c
	cc $(CFLAGS) -Iinclude -c -o .build/xoodyak-internal.o src/xoodyak_internal.c


.PHONY: all test clean

test: keyed-xoodyak.so
	cc $(TEST_CFLAGS) -Iinclude -o .build/test test/main.c src/*.c
	.build/test
	cc $(TEST_CFLAGS) -Iinclude -o .build/test test/main.c ./keyed-xoodyak.so
	.build/test

clean:
	rm -rf .build
	rm -f xoodyak.so
	rm -f keyed-xoodyak.so


_ := $(shell mkdir -p .build)
