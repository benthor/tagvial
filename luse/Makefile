include config.mak

.PHONY:all
all:build

.PHONY:build clean
build clean:
	$(MAKE) -C src -f luse.mak $@
	$(MAKE) -C src -f userdata.mak $@
	$(MAKE) -C src -f errno.mak $@
	$(MAKE) -C src -f posixio.mak $@

.PHONY:install
install:build
	mkdir -p $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(LUSE)))
	cp src/$(notdir $(subst .,/,$(LUSE))).so $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(LUSE)))
	mkdir -p $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(ERRNO)))
	cp src/$(notdir $(subst .,/,$(ERRNO))).so $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(ERRNO)))
	mkdir -p $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(USERDATA)))
	cp src/$(notdir $(subst .,/,$(USERDATA))).so $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(USERDATA)))
	mkdir -p $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(POSIXIO)))
	cp src/$(notdir $(subst .,/,$(POSIXIO))).so $(INSTALL_TOP_LIB)/$(dir $(subst .,/,$(POSIXIO)))

.PHONY:uninstall
uninstall:
	rm -f $(INSTALL_TOP_LIB)/$(subst .,/,$(LUSE)).so
	rm -f $(INSTALL_TOP_LIB)/$(subst .,/,$(ERRNO)).so
	rm -f $(INSTALL_TOP_LIB)/$(subst .,/,$(USERDATA)).so
	rm -f $(INSTALL_TOP_LIB)/$(subst .,/,$(POSIXIO)).so

.PHONY:doc
doc:
	$(MAKE) -C doc

