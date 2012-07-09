# tagvial #
*a trivial tagging filesystem*

## Notes ##

#### The implementation is woefully incomplete, do not use! ####
- (That said, the basics are already working)
- You see here only the fruits of about three evenings of deep-hacking-mode so far, way more to come.

## Overview ##

tagvial is a filesystem in userspace which replaces the traditional hirarchical view of directory trees with that of flat tag clouds (i.e., directories work now as tags)

The user creates a directory in the root of the filesystem for each tag she wants to use. Each directory is accessible from "within" each other directory. Files are tagged by copying them into one or more directories. For adding multiple tags to the file, the user can simply copy it to the end of a long path of tag-directories. The file will get tagged with the names of all directories in this path. It is subsequently accessible from any of these directories.

(For now) every file name can only exist once in the file system. "File exists"-errors are raised at appropriate moments

*To be continued ...*

### Working Features ###

- adding tags via mkdir
- removing tags vir rmdir
    - also works with non-empty directories
- undelete of tags (by doing mkdir of previously rmdir'd tag)
    - can be disabled with `-o noundelete`
- renaming of files
- removing of files from tags or completely (via 'rm')
    - default behavior: rm'ing a file in a directory only removes it from that tag. rm'ing in the root is denied
    - `-o allowrootrm`: removing files from the root actually deletes them
    - `-o deletetagless` will automatically remove untagged file
- retagging of files via "rename" (i.e., "mv")
- appropriate errors raised if naming clashes occur in files or directories
- proper unmounting is actually not needed
    - persistent database (as plain .lua file in backend directory), gets updated on every transaction
    - all files stored in a single "normal" dir in the backed

#### mount options ####

tagvial's behavior is almost exclusively determined by its various mount options. Care has been taken to chose sane/conservative defaults, the below options result in a more interesting and/or more dangerous experience.

- `-o noundelete`
    - by default, rmdir'ing a directory/tag only "hides" the tag, mkdiring it again under the same name restores the tag to all files. This can be disabled by the above option

- `-o recursiverm`
    - by default, rm'ing a file from a tagpath only removes the last tag. With this option it instead loses all tags from its entire tagpath.

- `-o recursivermdir`
    - by default, rmdir'ing a directory/tag only removes that particular tag. By specifying the above option, all other tags in the path between the tag and the root also get deleted.
    - *bug/unexpected behavior*: currently this also happenes when your current working directory is a tagpath within the fs. For example, with this option, when you are currently in `mountpoint/important/todo/pdfs` and decide to delete a random other tag, the tags `important`, `todo` and `pdfs` get deleted as well. You have been warned
    - *bug/unexpected behavior*: currently doesn't respect `-o deletetagless` directive

- `-o allowrootrm`
    - by default, files under the root are undeletable. This option changes this and will obviously also remove all tags from the file.

- `-o deletetagless`
    - by default, a file that does not have any tags is kept in the root. This mount option will automatically clean away a file once it is removed from its last tag
    - *feature/unexpected behavior*: this _only_ happens for files which are explicitly 'rm'd. 'rmdir'ing all its tags does not lead to file deletion

### Most Important Non-working Stuff ###

- copy of an already tagged file into a directory doesn't add the tag
    - will produce a "file exists"-error
    - for now use 'mv'
- no links, no proper file attributes, no permissions

### Credits ###

tagvial is implemented using Jérôme Vuarand's excellent [fuse](http://fuse.sourceforge.net/) [bindings](http://luse.luaforge.net/) for [Lua](http://lua.org) (included here for convenience). I ([benthor](https://github.org/benthor)) heavily based this alpha implementation on some [example code](http://luse.luaforge.net/fwfs.lua) of his.

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
- loads of stuff

## Bugs ##
- also probably loads, although your data should about as safe as your backend filesystem
    - also, if you neither specify `-o deletetagless` nor `-o allowrootrm`, you _can't accidentally delete a file_
    - if you don't specify `-o noundelete`, even your tags are safe
