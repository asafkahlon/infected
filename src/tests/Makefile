IDIR =../../include

CC = /usr/bin/gcc
CFLAGS = -I$(IDIR) -fPIC -Wall -Wextra -g # C flags
LDFLAGS = -L./../ -linfected_decoder -lcunit # linking flags

test: test.c
	$(CC) -o $@ $< $(CFLAGS) $(LDFLAGS)

.PHONY: all
all: test
	DYLD_LIBRARY_PATH=./../ ./test

.PHONY: clean
clean:
	rm -f *.o
	rm -f test