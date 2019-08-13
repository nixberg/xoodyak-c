CFLAGS= -fPIC -Os -Wall -Werror -Wextra -std=c99
TEST_CFLAGS= -g3 -fPIC -Wall -Werror -Wextra -std=c99 -fsanitize=address,undefined

.PHONY: all test clean

all: xoodyak.so

xoodyak.so: obj/xoodyak.o obj/xoodoo.o
	$(CC) $(CFLAGS) -shared obj/xoodyak.o obj/xoodoo.o -o xoodyak.so

obj/xoodyak.o: src/xoodyak.c src/xoodyak.h src/xoodoo.h
	$(CC) $(CFLAGS) src/xoodyak.c -c -o obj/xoodyak.o

obj/xoodoo.o: src/xoodoo.c src/xoodyak.h
	$(CC) $(CFLAGS) src/xoodoo.c -c -o obj/xoodoo.o

test:
	@$(CC) $(TEST_CFLAGS) src/xoodoo.c -c -o obj/test_xoodoo.o
	@$(CC) $(TEST_CFLAGS) src/xoodyak.c -c -o obj/test_xoodyak.o
	@$(CC) $(TEST_CFLAGS) tests/main.c -c -o obj/test.o -I lib
	@$(CC) $(TEST_CFLAGS) obj/test.o obj/test_xoodyak.o obj/test_xoodoo.o -o obj/test
	@./obj/test

clean:
	rm -r obj

_ := $(shell mkdir -p obj)
