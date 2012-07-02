# Installation directory
INSTALL_TOP_SHARE=/usr/share/lua/5.1
INSTALL_TOP_LIB=/usr/lib/lua/5.1

# Paths to Lua, if not in standard paths
#CPPFLAGS+=-I/some/path/to/lua-5.1.2/include
#LDFLAGS+=-L/some/path/to/lua-5.1.2/lib

CPPFLAGS+=-fPIC
CFLAGS+=-fPIC

# FUSE version. You can compile for different FUSE versions. Supported
# versions are: 25, 26.
FUSE_VERSION=26

# Module name. You can change the module names and hierarchy here.
LUSE=luse
ERRNO=luse.errno
USERDATA=luse.userdata
POSIXIO=luse.posixio

# LUSE version. Do not change it.
LUSE_VERSION="1.0.3"

