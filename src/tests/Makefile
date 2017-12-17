PROJDIR = ../..
IDIR = $(PROJDIR)/include

CC = /usr/bin/gcc
CFLAGS = -I$(IDIR) -I$(PROJDIR)/src -fPIC -Wall -Wextra -g # C flags
LDFLAGS = -L$(PROJDIR)/src -linfected_decoder -lcunit # linking flags

SUITES = general.o barker.o
MAIN = main.out

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(MAIN): main.c tests.h $(SUITES)
	$(CC) -o $@ $< $(SUITES) $(CFLAGS) $(LDFLAGS)

.PHONY: all
all: $(MAIN)
	DYLD_LIBRARY_PATH=$(PROJDIR)/src ./$(MAIN)

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(MAIN)