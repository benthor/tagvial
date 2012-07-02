include ../config.mak

CPPFLAGS=-Dluaopen_module=luaopen_$(subst .,_,$(ERRNO))
LDFLAGS=-shared
LDLIBS=-llua

build:errno.so

errno.so:errno.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -f errno.o errno.so
