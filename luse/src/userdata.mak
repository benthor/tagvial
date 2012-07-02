include ../config.mak

CPPFLAGS=-Dluaopen_module=luaopen_$(subst .,_,$(USERDATA))
LDFLAGS=-shared
LDLIBS=-llua

build:userdata.so

userdata.so:userdata.o
	$(CC) $(CPPFLAGS) $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)

clean:
	rm -f userdata.o userdata.so
