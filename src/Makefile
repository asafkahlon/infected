IDIR =../include
ODIR=obj

DESTDIR=/usr/local

CFLAGS += -I$(IDIR) -fPIC -Wall -Wextra -O2 -g # C flags
LDFLAGS = -shared  # linking flags

LIBDECODER = libinfected_decoder.so

_DEPS = infected_decoder.h log.h
DEPS := $(patsubst %,$(IDIR)/%,$(_DEPS))
DEPS += infected_decoder_private.h

_OBJS = infected_decoder.o
OBJS := $(patsubst %,$(ODIR)/%,$(_OBJS))

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBDECODER): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

all: $(LIBDECODER)

.PHONY: install
install: $(LIBDECODER)
	install -d $(DESTDIR)/lib/
	install -v $(LIBDECODER) $(DESTDIR)/lib/$(LIBDECODER)
	install -d $(DESTDIR)/include/infected
	install -v $(IDIR)/*.h $(DESTDIR)/include/infected

.PHONY: uninstall
uninstall:
	rm -vf $(DESTDIR)/lib/$(LIBDECODER)
	rm -vrf $(DESTDIR)/include/infected

.PHONY: tests
tests: $(LIBDECODER)
	$(MAKE) -C $@ clean all

.PHONY: clean
clean:
	rm -vf $(ODIR)/*.o *~ core $(INCDIR)/*~ 