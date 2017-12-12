IDIR =../include
ODIR=obj

MAKE = make
CC = /usr/local/Cellar/gcc/7.2.0/bin/gcc-7 # C compiler
CFLAGS = -I$(IDIR) -fPIC -Wall -Wextra -O2 -g # C flags
LDFLAGS = -shared  # linking flags

LIBDECODER = libinfected_decoder.so

_DEPS = infected_decoder.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJS = infected_decoder.o
OBJS = $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBDECODER): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

test: $(LIBDECODER)
	$(MAKE) -C tests/ clean all

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 