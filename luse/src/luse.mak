include ../config.mak

CPPFLAGS=-Dluaopen_module=luaopen_$(subst .,_,$(LUSE)) -DFUSE_USE_VERSION=$(FUSE_VERSION) -DLUSE_VERSION='$(LUSE_VERSION)'
LDFLAGS=-shared
LDLIBS=-lfuse -llua -lm

build:luse.so

luse.so:posix_structs.o

luse.o:posix_structs.h

%.so:%.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -f posix_structs.o luse.o luse.so
