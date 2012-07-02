-- :TODO: change it to utf-8 once luaforge support it
local charset = ([[
vi: encoding=iso-8859-1
]]):sub(14, -2):upper()

require 'markdown'

local file_index = "index.html"
local file_manual = "manual.html"
local file_examples = "examples.html"

local function manlink(name)
	return '<a href="http://www.lua.org/manual/5.1/manual.html#pdf-'..name..'"><code>'..name..'</code></a>'
end

local function manclink(name)
	return '<a href="http://www.lua.org/manual/5.1/manual.html#'..name..'"><code>'..name..'</code></a>'
end

------------------------------------------------------------------------------

function print(...)
	local t = {...}
	for i=1,select('#', ...) do
		t[i] = tostring(t[i])
	end
	io.write(table.concat(t, '\t')..'\n')
end

function header()
	print([[
<?xml version="1.0" encoding="]]..charset..[["?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en"
lang="en">
<head>
<title>LUSE</title>
<meta http-equiv="Content-Type" content="text/html; charset=]]..charset..[["/>
<link rel="stylesheet" href="doc.css" type="text/css"/>
</head>
<body>
]])
	print([[
<div class="chapter" id="header">
<img width="128" height="128" alt="LUSE" src="luse.png"/>
<p>A FUSE binding for Lua</p>
<p class="bar">
<a href="]]..file_index..[[">home</a> &middot;
<a href="]]..file_index..[[#download">download</a> &middot;
<a href="]]..file_index..[[#installation">installation</a> &middot;
<a href="]]..file_manual..[[">manual</a> &middot;
<a href="]]..file_examples..[[">examples</a>
</p>
</div>
]])
end

function footer()
	print([[
<div class="chapter" id="footer">
<small>Last update: ]]..os.date"%Y-%m-%d %H:%M:%S %Z"..[[</small>
</div>
]])
	print[[
</body>
</html>
]]
end

local chapterid = 0

function chapter(title, text, sections, raw)
	chapterid = chapterid+1
	local text = text:gsub("%%chapterid%%", tostring(chapterid))
	if not raw then
		text = markdown(text)
	end
	if sections then
		for _,section in ipairs(sections) do
			section = section:gsub("%%chapterid%%", tostring(chapterid))
			text = text..[[
<div class="section">
]]..markdown(section)..[[
</div>]]
		end
	end
	print([[
<div class="chapter">
<h1>]]..tostring(chapterid).." - "..title..[[</h1>
]]..text..[[
</div>
]])
end

------------------------------------------------------------------------------

io.output(io.open(file_index, "wb"))

header()

chapter("About LUSE", [[
LUSE is a Lua binding for [FUSE](http://fuse.sourceforge.net/), which is a library allowing creation filesystem drivers run in userspace. LUSE is a low level binding. FUSE itself it rather slow, so LUSE tries not to impede performance more than necessary. For that reason it is manipulating userdata and pointers directly rather than Lua strings, with the reduced safety implied.

LUSE tries to be complete, but is not supporting obsolete APIs. The binding is closely following the FUSE API, so in most case you can use FUSE documentation if the present page is not clear enough. The missing functions are missing because I've not used them yet. I can add them on request (a use case could be helpful for non-trivial ones).

LUSE module itself is very basic, so I developed several other modules to help the development of a complete filesystem.

*errno* contains many constants representing POSIX error numbers. It also have an accessor to query the errno variable (which contains the last error number).

*userdata* can create and manipulate untyped userdata, to be used as buffers for read and write commands of the filesystem.

*posixio* is not strictly necessary, but it is very useful to implement many types of filesystem, which redirect I/O request to another filesystem. It contains bindings to many file and directory manipulation functions. Here again the read and write functions manipulate untyped userdata to keep overhead minimal.

## Support

All support is done through the [Lua mailing list](http://www.lua.org/lua-l.html). If the traffic becomes too important a specialized mailing list will be created.

Feel free to ask for further developments. I can't guarantee that I'll develop everything you ask, but I want my code to be as useful as possible, so I'll do my best to help you. You can also send me request or bug reports (for code and documentation) directly at [jerome.vuarand@gmail.com](mailto:jerome.vuarand@gmail.com).

## Credits

This module is written and maintained by Jérôme Vuarand. It is inspired by [luafuse](http://code.google.com/p/luafuse/) module by Gary Ng, but it has been rewritten completely from scratch.

This website, LUSE downloadable packages and LUSE mailing list are generously hosted by [Luaforge.net](http://luaforge.net/). Consider making a donation.

LUSE is available under a [MIT-style license](LICENSE.txt).
]])

chapter('<a name="download">Download</a>', [[
LUSE sources are available in its [Mercurial repository](http://piratery.net/hg/luse/):

    hg clone http://piratery.net/hg/luse/

Tarballs of the latest code can be downloaded directly from there: as [gz](http://piratery.net/hg/luse/archive/tip.tar.gz), [bz2](http://piratery.net/hg/luse/archive/tip.tar.bz2) or [zip](http://piratery.net/hg/luse/archive/tip.zip).

LUSE is also available on its [Luaforge project page](http://luaforge.net/frs/?group_id=303).
]])

chapter('<a name="installation">Installation</a>', [[
To build LUSE edit config.mak and then run make in the top directory:

    $ vi config.mak
    $ make
    $ make install

LUSE obviously depends on FUSE. It currently supports FUSE API versions 25 (FUSE 2.5.x and later) and 26 (FUSE 2.6.x and later). You can select a FUSE API version in the config.mak file.
]])

footer()

------------------------------------------------------------------------------

io.output(io.open(file_manual, "wb"))

header()

local functions = { {
	name = "luse_functions";
	title = "luse module";
	doc = "These functions are global to the LUSE module.";
	functions = { {
		name = "luse.main";
		parameters = {"argv", "fs"};
		doc = [[
Starts a new filesystem daemon. `argv` is an array containing additionnal parameters to pass to the FUSE library. `fs` is a table (or any indexable object) containing methods of the FUSE filesystem you are trying to create.

    local luafs = {}
    function luafs:getattr(path, stat) return -errno.ENOSYS end
    local argv = {"luafs", select(2, ...)}
    luse.main(argv, fs)
]];
	}, {
		name = "luse.get_context";
		parameters = {"[tbl]"};
		doc = [[Obtain information about the context of the current FUSE request. A table is returned containing fields `uid`, `gid` and `pid` (user, group and thread id of the calling process, respectively). If no arguments are given, a new table is created. If a table is passed as an argument, it is reused and fields are set in it instead.]];
	}
} }, {
	name = "fuse_functions";
	title = "Filesystem methods";
	doc = [[These methods may be present in the `fs` object passed to `luse.main`. They are all optionnal, though without them your filesystem may not work properly. See example filesystems available in LUSE packages for minimal requirements.

Unless otherwise noted, all these functions should return 0 on success. On error, they should returned a negated error number (for example `-errno.EINVAL`, see errno module below). Each function will receive the object passed to `luse.main` as first parameter, that's why they are documented with the colon notation below.

As of FUSE 2.6, there are six FUSE methods that are not bound by LUSE: and `init`, `destroy`, `lock` and `bmap` which I haven't used yet and are a bit complicated to bind, and `getdir` and `utime` which are obsolete.]];
	functions = { {
		name = "fs:getattr";
		parameters = {"path", "st"};
		doc = [[Get file attributes. `st` is a `stat` structure as described in the `posixio.new` function below. The `'dev'` and `'blksize'` fields are ignored. The `'ino'` field is ignored except if the `'use_ino'` mount option is given.
		
Note that for the *find* command line utility to work in your filesystem, the `'nlink'` field of the `stat` structure must be properly filled. See [this FUSE FAQ entry](http://fuse.sourceforge.net/wiki/index.php/FAQ#Why_doesnx27.t_find_work_on_my_filesystemx3f.) for more details.]];
	},{
		name = "fs:readlink";
		parameters = {"path", "buf", "size"};
		doc = [[
Read the target of a symbolic link. The userdata buffer `buf` should be filled with a null terminated string. You can use `userdata.memcpy` to write the content of a string to it.

    function luafs:readlink(path, buf, size)
        local link = "/foo"
        userdata.memcpy(buf, link, math.min(size-1, #link)+1)
    end
]];
	},{
		name = "fs:mknod";
		parameters = {"path", "mode", "redev"};
		doc = [[Create a file node. This is called for creation of all non-directory, non-symlink nodes. If the filesystem defines a `create` method, then for regular files that will be called instead. `mode` is a Lua set (see description of `stat` structure in `posixio.new` documentation).]];
	},{
		name = "fs:mkdir";
		parameters = {"path", "mode"};
		doc = [[Create a directory.]];
	},{
		name = "fs:unlink";
		parameters = {"path"};
		doc = [[Remove a file.]];
	},{
		name = "fs:rmdir";
		parameters = {"path"};
		doc = [[Remove a directory.]];
	},{
		name = "fs:symlink";
		parameters = {"linkname", "path"};
		doc = [[Create a symbolic link. `path` is the name of the link itself in the LUSE filesystem, `linkname` is the linked file.]];
	},{
		name = "fs:rename";
		parameters = {"oldpath", "newpath"};
		doc = [[Rename a file.]];
	},{
		name = "fs:link";
		parameters = {"oldpath", "newpath"};
		doc = [[Create a hard link to a file.]];
	},{
		name = "fs:chmod";
		parameters = {"path", "mode"};
		doc = [[Change the permission bits of a file. `mode` is a Lua set (see description of `stat` structure in `posixio.new` documentation).]];
	},{
		name = "fs:chown";
		parameters = {"path", "uid", "gid"};
		doc = [[Change the owner and group of a file.]];
	},{
		name = "fs:truncate";
		parameters = {"path", "size"};
		doc = [[Change the size of a file.]];
	},{
		name = "fs:open";
		parameters = {"path", "fi"};
		doc = [[
File open operation. `fi` is a `fuse_file_info` structure (see below). No creation, or truncation flags (`'CREAT'`, `'EXCL'`, `'TRUNC'`) will be passed to `fs:open`. Open should check if the operation is permitted for the given flags. Optionally open may also return an arbitrary file handle in the fuse\_file\_info structure, which will be passed to all subsequent operations on that file.

The `fuse_file_info` structure has the following members:

- `flags`, table, this table is a lua set, see the documentation of `posixio.open`
- `writepage`, number
- `direct_io`, boolean
- `keep_cache`, boolean
- `flush`, boolean
- `fh`, number, the file handle that the filesystem can write to
- `lock_owner`, number
]];
	},{
		name = "fs:read";
		parameters = {"path", "buf", "size", "offset", "fi"};
		doc = [[
Read data from an open file. `buf` is a lightuserdata pointing to a buffer of size `size`. `offset` is a position inside the file.

Read should return exactly the number of bytes requested except on end of file or on error, otherwise the rest of the data will be substituted with zeroes. An exception to this is when the `direct_io` mount option is specified, in which case the return value of the read system call will reflect the return value of this operation.]];
	},{
		name = "fs:write";
		parameters = {"path", "buf", "size", "offset", "fi"};
		doc = [[
Write data to an open file. `buf` is a lightuserdata pointing to a buffer of size `size`. `offset` is a position inside the file.

Write should return exactly the number of bytes requested except on error. An exception to this is when the `direct_io` mount option is specified (see read operation).]];
	},{
		name = "fs:statfs";
		parameters = {"path", "st"};
		doc = [[Get file system statistics. `st` is a `statvfs` structure (see `posixio.new` documentation).The `'frsize'`, `'favail'`, `'fsid'` and `'flag'` fields are ignored.]];
	},{
		name = "fs:flush";
		parameters = {"path", "fi"};
		doc = [[
Possibly flush cached data.

BIG NOTE: This is not equivalent to fsync(). It's not a request to sync dirty data.

Flush is called on each `fs:close` of a file descriptor. So if a filesystem wants to return write errors in `fs:close` and the file has cached dirty data, this is a good place to write back data and return any errors. Since many applications ignore `fs:close` errors this is not always useful.

NOTE: The `fs:flush` method may be called more than once for each `fs:open`. This happens if more than one file descriptor refers to an opened file due to dup(), dup2() or fork() calls. It is not possible to determine if a flush is final, so each flush should be treated equally. Multiple write-flush sequences are relatively rare, so this shouldn't be a problem.

Filesystems shouldn't assume that `fs:flush` will always be called after some writes, or that if will be called at all.
]];
	},{
		name = "fs:release";
		parameters = {"path", "fi"};
		doc = [[
Release an open file.

Release is called when there are no more references to an open file: all file descriptors are closed and all memory mappings are unmapped.

For every open() call there will be exactly one release() call with the same flags and file descriptor. It is possible to have a file opened more than once, in which case only the last release will mean, that no more reads/writes will happen on the file. The return value of release is ignored.
]];
	},{
		name = "fs:fsync";
		parameters = {"path", "datasync", "fi"};
		doc = [[Synchronize file contents. If the `datasync` parameter is `true`, then only the user data should be flushed, not the meta data.]];
	},{
		name = "fs:setxattr";
		parameters = {"path", "name", "value", "size", "flags"};
		doc = [[Set extended attributes.]];
	},{
		name = "fs:getxattr";
		parameters = {"path", "name", "value", "size", "flags"};
		doc = [[Get extended attributes.]];
	},{
		name = "fs:listxattr";
		parameters = {"path", "list", "size"};
		doc = [[List extended attributes.]];
	},{
		name = "fs:removexattr";
		parameters = {"path", "name"};
		doc = [[Remove extended attributes.]];
	},{
		name = "fs:opendir";
		parameters = {"path", "fi"};
		doc = [[Open directory. This method should check if the open operation is permitted for this directory.]];
	},{
		name = "fs:readdir";
		parameters = {"path", "filler", "off", "fi"};
		doc = [[
Read directory. `filler` is a function with the prototype `filler(name, off, fi)` that must be called for each directory entry.

The filesystem may choose between two modes of operation:

1. The readdir implementation ignores the offset parameter, and passes zero to the filler function's offset. The filler function will not return 1 (unless an error happens), so the whole directory is read in a single readdir operation.
1. The readdir implementation keeps track of the offsets of the directory entries. It uses the offset parameter and always passes non-zero offset to the filler function. When the buffer is full (or an error happens) the filler function will return 1.

NOTE: This LUSE binding currently only support the first mode of operation. You must pass `nil` as `off` parameter to the `filler` function.
]];
	},{
		name = "fs:releasedir";
		parameters = {"path", "fi"};
		doc = [[Release directory.]];
	},{
		name = "fs:fsyncdir";
		parameters = {"path", "datasync", "fi"};
		doc = [[Synchronize directory contents. If the `datasync` parameter is `true`, then only the user data should be flushed, naot the meta data.]];
	},{
	--[=[
		name = "fs:init";
		parameters = {};
		doc = [[Initialize filesystem. The return value will passed in the private\_data field of fuse\_context to all file operations and as a parameter to the fs:destroy method.]];
	},{
		name = "fs:destroy";
		parameters = {"conn"};
		doc = [[Clean up filesystem. Called on filesystem exit.]];
	},{
	--]=]
		name = "fs:access";
		parameters = {"path", "mask"};
		doc = [[
Check file access permissions. This will be called for the `access()` system call. If the `'default_permissions'` mount option is given, this method is not called.

This method is not called under Linux kernel versions 2.4.x.
]];
	},{
		name = "fs:create";
		parameters = {"path", "mode", "fi"};
		doc = [[
Create and open a file.

If the file does not exist, first create it with the specified mode, and then open it.

If this method is not implemented or under Linux kernel versions earlier than 2.6.15, the mknod() and open() methods will be called instead.
]];
	},{
		name = "fs:ftruncate";
		parameters = {"path", "size", "fi"};
		doc = [[
Change the size of an open file.

This method is called instead of the `fs:truncate` method if the truncation was invoked from an `ftruncate()` system call.

If this method is not implemented or under Linux kernel versions earlier than 2.6.15, the `fs:truncate` method will be called instead.
]];
	},{
		name = "fs:fgetattr";
		parameters = {"path", "st", "fi"};
		doc = [[
Get attributes from an open file.

This method is called instead of the `fs:getattr` method if the file information is available.

Currently this is only called after the `fs:create` method if that is implemented (see above). Later it may be called for invocations of `fstat()` too.
]];
	--[=[
	},{
		name = "fs:lock";
		parameters = {"path", "fi", "cmd", "lock"};
		doc = [[]];
	--]=]
	},{
		name = "fs:utimens";
		parameters = {"path", "tv"};
		doc = [[
Change the access and modification times of a file with nanosecond resolution. `tv` is a Lua array containing two other tables, the new access time and modification time respectively. These subtables have two fields each:

- `sec`, number, time in seconds
- `nsec`, number, the sub-second portion of the time, in nanoseconds
]];
	--[=[
	},{
		name = "fs:bmap";
		parameters = {};
		doc = [[]];
	--]=]
	}
} }, {
	name = "errno_functions";
	title = "errno module";
	doc = [[The module contains a number of constants taken from various C, POSIX and Linux headers. Not all error codes are there, but the most common for filesystem operation are defined. Ask if you need more. The bound error codes are: EPERM, ENOENT, ESRCH, EINTR, EIO, ENXIO, E2BIG, ENOEXEC, EBADF, ECHILD, EAGAIN, ENOMEM, EACCES, EFAULT, ENOTBLK, EBUSY, EEXIST, EXDEV, ENODEV, ENOTDIR, EISDIR, EINVAL, ENFILE, EMFILE, ENOTTY, ETXTBSY, EFBIG, ENOSPC, ESPIPE, EROFS, EMLINK, EPIPE, EDOM, ERANGE, ENOSYS.]];
	functions = { {
		name = "errno.errno";
		doc = [[This pseudo-variable is a getter that can retrieve the last C errno number. It can be compared to other errno constants, or returned to LUSE.]];
	},{
		name = "errno.strerror";
		parameters = {"err"};
		doc = [[Returns a string corresponding to the error number `err`.]];
	}
} }, {
	name = "userdata_functions";
	title = "userdata module";
	doc = "This module exposes some functions to manipulate userdata. It also provides a generic way to create a buffer userdata (without metatable or environment).";
	functions = { {
		name = "userdata.memcpy";
		parameters = {"to", "from", "size"};
		doc = [[Writes data to a userdata. `to` must be a userdata (full or light). `from` can be a userdata or a string. `size` is the number of bytes copied. No check of the destination or source size is made (it's impossible for light userdata), so this function may read past data end or overwrite memory. Use it with care.]];
	},{
		name = "userdata.new";
		parameters = {"size"};
		doc = [[
Creates a new userdata buffer of the specified `size`. This is a full userdata allocated by Lua. It has two fields: `data`, which is a lightuserdata containing the address of the buffer, and size, which is a number containing the size of the buffer. You can also index the buffer with a number key to offset the buffer address (0-based offset). The userdata also have a `__tostring` metamethod which converts the content of the userdata to a Lua string.

    local ud = userdata.new(2) -- create a 2 byte userdata
    print(ud.size) -- size of userdata (2)
    print(ud.data) -- address of first byte
    print(ud[1]) -- address of second byte
    print(tostring(ud)) -- print content of the userdata
]];
	},{
		name = "userdata.tostring";
		parameters = {"ud"};
		doc = [[This functions converts the content of the full userdata `ud` to a string.]];
	}
} }, {
	name = "posixio_functions";
	title = "posixio module";
	doc = [[
This module provides some function to manipulate files and directories. It's a light binding over the POSIX API for files and directories. Unless otherwise stated, these functions return an errno error number. You can compare them with constants in the `errno` module or return them to LUSE.

Except for posixio.new, all files are direct binding to POSIX functions. You can get their documentation through your system manual. Some portions of a Linux manual have been copied here.
]];
	functions = { {
		name = "posixio.new";
		parameters = {"type [", "count]"};
		doc = [[
Allocates a POSIX structure. Type must be the name of a supported POSIX struct among: `'stat'`, `'statvfs'` and `'timeval'`. `count` can be used to allocate more than one structure. This is currently only supported with `timeval` structures.

The `stat` structure has the following members:

- `dev`, number, ID of device containing file
- `ino`, number, inode number
- `mode`, table, protection; this table is a lua set, with the following keys possibly present and true: `IFBLK`, `IFCHR`, `IFIFO`, `IFREG`, `IFDIR`, `IFLNK`, `IFSOCK`, `IRUSR`, `IWUSR`, `IXUSR`, `IRGRP`, `IWGRP`, `IXGRP`, `IROTH`, `IWOTH`, `IXOTH`, `ISUID`, `ISGID`, `ISVTX`.
- `nlink`, number, number of hard links
- `uid`, number, user ID of owner
- `gid`, number, group ID of owner
- `rdev`, number, device ID (if special file)
- `size`, number, total size, in bytes
- `blksize`, number, blocksize for filesystem I/O
- `blocks`, number, number of blocks allocated
- `atime`, number, time of last access
- `mtime`, number, time of last modification
- `ctime`, number, time of last status change

The `statvfs` structure has the following members:

- `bsize`, number, file system block size
- `frsize`, number, fragment size
- `blocks`, number, size of fs in f_frsize units
- `bfree`, number, # free blocks
- `bavail`, number, # free blocks for non-root
- `files`, number, # inodes
- `ffree`, number, # free inodes
- `favail`, number, # free inodes for non-root
- `fsid`, number, file system ID
- `flag`, number, mount flags
- `namemax`, number, maximum filename length

The `timeval` structure has the following members:

- `sec`, number, seconds
- `usec`, number, microseconds
]];
	},{
		name = "posixio.stat";
		parameters = {"path", "st"};
		doc = [[Get file status. `path` is the path to a file. `st` is a `stat` userdata created with `posixio.new('stat')`.]];
	},{
		name = "posixio.fstat";
		parameters = {"fd", "st"};
		doc = [[Get file status. `fd` is the fd of an open file. `st` is a `stat` structure that is filled by `posixio.stat`.]];
	},{
		name = "posixio.lstat";
		parameters = {"path", "st"};
		doc = [[`posixio.lstat` is identical to `posixio.stat`, except that if `path` is a symbolic link, then the link itself is stated, not the file that it refers to.]];
	},{
		name = "posixio.opendir";
		parameters = {"path"};
		doc = [[
Open a directory.

Returns nil on error, or a `directory` userdata on success. See below for the methods of this userdata.
]];
	},{
		name = "directory:readdir";
		parameters = {};
		doc = [[
Read a file entry from a directory.

Returns nil on error or end of directory listing, or a `dirent` structure (as a userdata) on success. The `dirent` structure has the following fields:

- `ino`, number, inode number
- `off`, number, offset to this dirent
- `reclen`, number, length of this `name`
- `name`, string, filename
]];
	},{
		name = "directory:closedir";
		parameters = {};
		doc = [[Close directory.]];
	},{
		name = "posixio.mkdir";
		parameters = {"path"};
		doc = [[Create a directory.]];
	},{
		name = "posixio.rmdir";
		parameters = {"path"};
		doc = [[Delete a directory.]];
	},{
		name = "posixio.mknod";
		parameters = {"path", "mode", "dev"};
		doc = [[Create a file. `mode` is a Lua set (see description of `stat` structure in `posixio.new` documentation). `dev` is the device number.]];
	},{
		name = "posixio.unlink";
		parameters = {"path"};
		doc = [[Delete a name and possible the file it refers to.]];
	},{
		name = "posixio.open";
		parameters = {"path", "flags"};
		doc = [[
Open a file. `flags` is a Lua set. The set must contain one of these three flags: `'RDWR'`, `'RDONLY'` or `'WRONLY'`. The following optionnal flags are also recognized: `APPEND`, `ASYNC`, `CREAT`, `DIRECT`, `DIRECTORY`, `EXCL`, `LARGEFILE`, `NOATIME`, `NOCTTY`, `NOFOLLOW`, `NONBLOCK`, `NDELAY`, `SYNC`, `TRUNC`.

Returns -1 on error, or a non-negative number on success. That number is a file descriptor and can be used in subsequent access to this file.
]];
	},{
		name = "posixio.close";
		parameters = {"fd"};
		doc = [[Close a file. `fd` is a file descriptor of an open file.]];
	},{
		name = "posixio.read";
		parameters = {"fd", "buf", "size"};
		doc = [[Read from a file descriptor. `posixio.read` attempts to read up to `size` bytes from file descriptor `fd` into the userdata buffer `buf`. `buf` can be a light or a full userdata; in either case no size check is performed, so use this function with care.]];
	},{
		name = "posixio.write";
		parameters = {"fd", "buf", "size"};
		doc = [[Write to a file descriptor. `posixio.write` writes up to `size` bytes to the file referenced by the descriptor `fd` from the userdata buffer `buf`. `buf` can be a light or a full userdata; in either case no size check is performed, so use this function with care.]];
	},{
		name = "posixio.lseek";
		parameters = {"fd", "offset", "whence"};
		doc = [[
Reposition read/write offset. The `posixio.lseek` function repositions the offset of the open file associated with the file descriptor fildes to the argument offset according to the directive whence as follows:

- `'SET'`: The offset is set to offset bytes.
- `'CUR'`: The offset is set to its current location plus offset bytes.
- `'END'`: The offset is set to the size of the file plus offset bytes.

The lseek() function allows the file offset to be set beyond the end of the file (but this does not change the size of the file). If data is later written at this point, subsequent reads of the data in the gap (a "hole") return null bytes ('\0') until data is actually written into the gap.
]];
	},{
		name = "posixio.statvfs";
		parameters = {"path", "st"};
		doc = [[Get file system statistics. The function `posixio.statvfs` returns information about a mounted file system. `path` is the pathname of any file within the mounted filesystem. `st` is a `statvfs` userdata created with `posixio.new('statvfs')`.]];
	},{
		name = "posixio.utimes";
		parameters = {"path", "times"};
		doc = [[
Change access and modification times of an inode, with resolution of 1 microsecond. `path` is the path of the file to change times, while `times` is an array of 2 `timeval` structures created with `posixio.new('timeval', 2)`.

Note that this function accepts microsecond resolution, while the `fs:utimens` method of a FUSE filesystem receives nanosecond resolution times. Make the appropriate conversion.
]];
	},{
		name = "posixio.rename";
		parameters = {"oldpath", "newpath"};
		doc = [[Change the name or location of a file.]];
	},{
		name = "posixio.getcwd";
		parameters = {"buf", "size"};
		doc = [[
Put the path of the current working directory in the userdata buffer `buf` of size `size`.

Returns the address of `buf` as a light userdata on success, a NULL light userdata on error.

    local buf = userdata.new(posixio.PATH_MAX)
    local result = posixio.getcwd(buf.data, buf.size)
    if result~=buf.data then error(errno.strerror(errno.errno)) end
]];
	},{
		name = "posixio.PATH_MAX";
		doc = [[This constant is the maximum number of characters allowed in a path on the current system. It may be useful to allocate some buffers passed to other `posixio` functions.]];
	}
} } }

local funcstr = ""
for sectionid,section in ipairs(functions) do
	funcstr = funcstr..[[
	<div class="section">
	<h2><a name="]]..section.name..[[">%chapterid%.]]..tostring(sectionid).." - "..section.title..[[</a></h2>
]]..markdown(section.doc)..[[

]]
	for _,func in ipairs(section.functions) do
		funcstr = funcstr..[[
		<div class="function">
		<h3><a name="]]..func.name..[["><code>]]..func.name
		if func.parameters then
			funcstr = funcstr..' ('..table.concat(func.parameters, ", ")..")"
		end
		funcstr = funcstr..[[</code></a></h3>
]]..markdown(func.doc)..[[
		</div>

]]
	end
	funcstr = funcstr..[[
	</div>

]]
end

local manual = [[
Here you can find a list of the functions present in the module and how to use them. LUSE main module follows Lua 5.1 package system, see the [Lua 5.1 manual](http://www.lua.org/manual/5.1/) for further explanations.

Quick links:

]]
for _,section in ipairs(functions) do
	manual = manual..'- ['..section.title..'](#'..section.name..')\n'
end
manual = markdown(manual)
manual = manual..funcstr
chapter('<a name="manual">Manual</a>', manual, nil, true)

footer()

------------------------------------------------------------------------------

io.output(io.open(file_examples, "wb"))

header()

chapter('<a name="examples">Examples</a>', [[
Here are some filesystem examples.`hellofs` is just a minimal example. `luafs` is a basic filesystem that exposes a Lua table as a directory. It can be used as a model to expose some data from a Lua state in your own applications. `fwfs` is a forwarding filesystem, that can be used as a base to make on the fly operations on file I/O.
]], { [[
## <a name="hellofs">%chapterid%.1 - hellofs</a>

`hellofs` is the Hello World! of FUSE filesystems. It creates a directory with a single file called `hello` that contain the string `"Hello World!"`.

Example:
    <pre>
    $ mkdir tmpdir
    $ ./hellofs.lua tmpdir
    $ ls tmpdir
    hello
    $ cat tmpdir/hello
    Hello World!
    $ fusermount -u tmpdir

Source code: [hellofs.lua](hellofs.lua)

]], [[
## <a name="luafs">%chapterid%.2 - luafs</a>

`luafs` expose a table as a directory. The subtables are exposed as subdirectories, while the string and userdata fields are exposed as regular files. You can create new files, and write to them: that will create new strings in the table hierarchy.

Example:
    $ mkdir tmpdir
    $ ./luafs.lua tmpdir
    $ ls tmpdir
    $ echo Hello World! > tmpdir/hello
    $ ls tmpdir
    hello
    $ cat tmpdir/hello
    Hello World!
    $ mkdir tmpdir/subdir
    $ ls tmpdir
    hello subdir/
    $ echo foo > tmpdir/subdir/bar
    $ ls tmpdir/subdir
    bar
    $ cat tmpdir/subdir/bar
    foo
    $ fusermount -u tmpdir

Source code: [luafs.lua](luafs.lua)

]], [[
## <a name="luafs">%chapterid%.3 - fwfs</a>

`fwfs` creates a directory that will forward all I/O to another directory. It's meant to be used as a base for any filesystem that is backed on disk, and that is doing something to the files on the fly. For example it can do filesystem encryption, it can make several directories on different disks appear as a single one, etc.

Example:
    $ mkdir srcdir
    $ echo Hello World! > srcdir/hello
    $ mkdir dstdir
    $ ./fwfs.lua srcdir dstdir
    $ ls dstdir
    hello
    $ cat dstdir/hello
    Hello World!
    $ mkdir dstdir/subdir
    $ ls srcdir
    hello subdir/
    $ echo foo > dstdir/subdir/bar
    $ ls srcdir/subdir
    bar
    $ cat srcdir/subdir/bar
    foo
    $ fusermount -u dstdir

Source code: [fwfs.lua](fwfs.lua)
]]})

footer()

------------------------------------------------------------------------------

-- vi: ts=4 sts=4 sw=4

