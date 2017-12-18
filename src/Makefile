IDIR =../include
ODIR=obj

CFLAGS = -I$(IDIR) -fPIC -Wall -Wextra -O2 -g # C flags
LDFLAGS = -shared  # linking flags

LIBDECODER = libinfected_decoder.so

_DEPS = infected_decoder.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJS = infected_decoder.o
OBJS := $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBDECODER): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: tests
tests: $(LIBDECODER)
	$(MAKE) -C $@ clean all

.PHONY: clean
clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 