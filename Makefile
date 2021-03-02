CFLAGS= -fPIC -Os -Wall -Werror -Wextra -std=c99
TEST_CFLAGS= -g3 -fPIC -Wall -Werror -Wextra -std=c99 -fsanitize=address,undefined

objects = .build/xoodoo.o .build/xoodyak.o .build/xoodyak-internal.o


.PHONY: all test clean


all: xoodyak.so keyed-xoodyak.so

xoodyak.so: $(objects)
	cc $(CFLAGS) -shared -o xoodyak.so $(objects)

keyed-xoodyak.so: $(objects) .build/keyed-xoodyak.o
	cc $(CFLAGS) -shared -o keyed-xoodyak.so $(objects) .build/keyed-xoodyak.o


.build/xoodoo.o: src/xoodoo.*
	cc $(CFLAGS) -DNDEBUG -c -o .build/xoodoo.o src/xoodoo.c

.build/xoodyak.o: src/*
	cc $(CFLAGS) -c -o .build/xoodyak.o src/xoodyak.c

.build/keyed-xoodyak.o: src/*
	cc $(CFLAGS) -c -o .build/keyed-xoodyak.o src/keyed_xoodyak.c

.build/xoodyak-internal.o: src/*
	cc $(CFLAGS) -c -o .build/xoodyak-internal.o src/xoodyak_internal.c


test: keyed-xoodyak.so .build/xoodoo.o
	cc $(TEST_CFLAGS) -o .build/test test/main.c src/*.c
	.build/test
	cc $(TEST_CFLAGS) -o .build/test test/main.c keyed-xoodyak.so
	.build/test


clean:
	rm -rf .build
	rm -f xoodyak.so
	rm -f keyed-xoodyak.so


_ := $(shell mkdir -p .build)
