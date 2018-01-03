PROJDIR = ../..
IDIR = $(PROJDIR)/include

CFLAGS += -I$(IDIR) -I$(PROJDIR)/src -fPIC -Wall -Wextra -g # C flags
LDFLAGS = -L$(PROJDIR)/src -linfected_decoder -lcunit # linking flags

DEPS = frames.h $(IDIR)/log.h
SUITES = general.o barker.o size.o frame.o
MAIN = main.out

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(MAIN): main.c $(SUITES)
	$(CC) -o $@ $< $(SUITES) $(CFLAGS) $(LDFLAGS)

.PHONY: all
all: $(MAIN)
	DYLD_LIBRARY_PATH=$(PROJDIR)/src LD_LIBRARY_PATH=$(PROJDIR)/src ./$(MAIN)

.PHONY: clean
clean:
	rm -f *.o
	rm -f $(MAIN)
