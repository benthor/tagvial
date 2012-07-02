include ../config.mak

CPPFLAGS=-Dluaopen_module=luaopen_$(subst .,_,$(POSIXIO))
LDFLAGS=-shared
LDLIBS=-llua

build:posixio.so

posixio.so:posix_structs.o

posixio.o:posix_structs.h

%.so:%.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -f posix_structs.o posixio.o posixio.so
