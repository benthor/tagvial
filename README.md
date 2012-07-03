# tagvial #
*a trivial tagging filesystem*

## Notes ##
- *The implementation is woefully incomplete, do not use!*
    - (That said, the basics are already working)
- You see here the fruits of about two evenings of *deep-hacking-mode* so far

## Overview ##

tagvial is a filesystem in userspace which replaces the traditional hirarchical view of directory trees with that of flat tag clouds (i.e., directories work now as tags)

The user creates a directory in the root of the filesystem for each tag he wants to use. Each directory is accessible from "within" each other directory. Files are tagged by copying them into one or more directories. For adding multiple tags to the file, the user can simply copy it to the end of a long path of tag-directories. The file will get tagged with the names of all directories in this path. It is subsequently accessible from any of these directories.

(For now) every file name can only exist once in the file system. "File exists"-errors are raised at appropriate moments

*To be continued ...*

### Working Features ###

- adding tags via mkdir
- removing tags vir rmdir
- undelete of tags (by doing mkdir of previously rmdir'd tag)
- renaming of files
- removing of files from tags or completely (via 'rm')
- retagging of files via "rename" (i.e., "mv")
- appropriate errors raised if naming clashes occur in files or directories
- persistent database (as plain .lua file in backend directory)

### Credits ###

tagvial is implemented using Jérôme Vuarand's excellent [fuse](http://fuse.sourceforge.net/) [bindings](http://luse.luaforge.net/) for [Lua](http://lua.org). I ([benthor](https://github.org/benthor)) heavily based this alpha implementation on some [example code](http://luse.luaforge.net/fwfs.lua) of his.

## Usage ##
*Much of this will change once I have implemented advanced configuraiton options*

### For Debugging ###

To see shiny colorful messages, run this (stays in the foreground)

    while test $! -eq 0; do lua tagvial.lua /path/to/backend_dir/ /path/to/mount_point/ -d; done

### Regular ###

The following mounts the stuff in the background

    lua tagvial.lua /path/to/backend_dir/ /path/to/mount_point/

### Unmounting ###

To unmount, use

    fusermount -zu /path/to/mount_point

## ToDo ##
- add a more conserviative default config
- loads
