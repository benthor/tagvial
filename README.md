# tagvial #
*a trivial tagging file system*

## Status ##

#### usable on alpha-level at your own risk####

(to date (Jul 11, 2012) , I've only worked on this for about four evenings, and this thing is accordingly still somewhat incomplete, see below)

## Overview ##

tagvial is a file system in userspace which replaces the traditional hirarchical view of directory trees with that of flat tag clouds (i.e., directories work now as tags)

The user creates a directory in the root of the file system for each tag she wants to use. Each directory is accessible from "within" each other directory. Files are tagged by copying them into one or more directories. For adding multiple tags to the file, the user can simply copy it to the end of a long path of tag-directories. The file will get tagged with the names of all directories in this path. It is subsequently accessible from any of these directories.

(For now) every file name can only exist once in the file system. "File exists"-errors are raised at appropriate moments

### Filesystem Semantics ###

- `mkdir` anywhere within the mouted file system (FS) creates a **tag** in the form of a directory which is then visible in any other "directory" in the FS apart from within itself

- `cp` of a file whose name does not exist in the FS yet adds it to all the tags in its destination path, as well as to the file system root. It is not possible to `cp` a filename which already exists in the root to an additional tag path, unless a new name is given. For adding a file to additional tags, use `mv` (at least for now)

- `mv` of a file will (for now) add it to all tags in the destination tag path, while removing all tags which are in the source path. I.e., `mv`ing a file from the root to a destination path adds all the latter's constituent tags to the file without removing any existing ones.

- `rm` of a file from the root is forbidden (by default, but see mount option `allowrootrm` below). `rm` of a file from a tag path removes it from the last tag in the path, or (with mount option `recursiverm`) from all tags in the path. If mount option `deletetagless` is given, a file is completely deleted if its last tag is removed.

- `rmdir` removes the last tag in the given path from the entire FS or, if the `recursivermdir` option is given, all tags in the path are removed. If files turn out to be tagless due to this, they are _not_ removed, regardless of potential `deletetagless` option. If the `noundelete` option isn't given, a subsequent `mkdir` of the removed tag name restores it and also readds all files that previously belonged to it.

- `ln` is not yet implemented

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
    - *note*: for now this option is required when using firefox to download a file directly into the fs, since it apparently does some "exploratory" mknod'ing and subsequent unlink'ing before attempting to store the file proper. 

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
- visible (directory) permissions are taken from the backend dir and can only be changed there. Although permission semantics are skewed in fuse anyway, so this might not work as expected
- file linking not working

### Credits ###

tagvial is implemented using Jérôme Vuarand's excellent [fuse](http://fuse.sourceforge.net/) [bindings](http://luse.luaforge.net/) for [Lua](http://lua.org) (included here for convenience). I ([benthor](https://github.org/benthor)) heavily based this alpha implementation on some [example code](http://luse.luaforge.net/fwfs.lua) of his.

## Usage ##
*Much of this will change once I have implemented advanced configuraiton options*

### For Debugging ###

To see shiny colorful messages, run this (stays in the foreground)

    while test $! -eq 0; do lua tagvial.lua /path/to/backend_dir/ /path/to/mount_point/ -o invalidmountoption,allowrootrm -d; done

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
    - also, if you neither specify `-o deletetagless` nor `-o allowrootrm`, you _can't accidentally delete a file_ at the mountpoint (backend is a diffent story of course)
    - if you don't specify `-o noundelete`, even your tags are safe (if you don't delete the backend database)
- possibly my mount-option names are a bit brain-dead
