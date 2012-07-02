#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#include "posix_structs.h"

/****************************************************************************/

static int lua__readdir(lua_State* L);
static int lua__closedir(lua_State* L);
static void lua__push__DIR(lua_State* L, DIR* dir)
{
	DIR** pdir;
	if (!dir)
	{
		lua_pushnil(L);
		return;
	}
	pdir = (DIR**)lua_newuserdata(L, sizeof(DIR*));
	*pdir = dir;
	lua_newtable(L);
	lua_newtable(L);
	lua_pushcfunction(L, lua__readdir);
	lua_setfield(L, -2, "readdir");
	lua_pushcfunction(L, lua__closedir);
	lua_setfield(L, -2, "closedir");
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__closedir);
	lua_setfield(L, -2, "__gc");
	lua_setmetatable(L, -2);
}

/****************************************************************************/

static int lua__new(lua_State* L)
{
	const char* type;
	size_t size = 1;
	
	type = luaL_checkstring(L, 1);
	if (!lua_isnoneornil(L, 2))
	{
		if (!lua_isnumber(L, 2)) return luaL_typerror(L, 2, "nil or number");
		size = lua_tonumber(L, 2);
	}
	
	if (!strcmp(type, "stat"))
	{
		if (size==1)
			lua__push__stat(L, 0);
		else
			lua_pushnil(L);
		return 1;
	}
	else if (!strcmp(type, "statvfs"))
	{
		if (size==1)
			lua__push__statvfs(L, 0);
		else
			lua_pushnil(L);
		return 1;
	}
	else if (!strcmp(type, "timeval"))
	{
		if (size==1)
			lua__push__timeval(L, 0);
		else
			lua__push__timeval_array(L, 0, size);
		return 1;
	}
	else
		return luaL_argerror(L, 1, "unrecognized struct type");
}

static int lua__stat(lua_State* L)
{
	const char* path;
	struct stat* buf;
	int result;
	
	path = luaL_checkstring(L, 1);
	if (!lua_isuserdata(L, 2)) return luaL_typerror(L, 2, "userdata");
	buf = *(struct stat**)lua_touserdata(L, 2);
	
	result = stat(path, buf);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__fstat(lua_State* L)
{
	int filedes;
	struct stat* buf;
	int result;
	
	filedes = luaL_checknumber(L, 1);
	if (!lua_isuserdata(L, 2)) return luaL_typerror(L, 2, "userdata");
	buf = *(struct stat**)lua_touserdata(L, 2);
	
	result = fstat(filedes, buf);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__lstat(lua_State* L)
{
	const char* path;
	struct stat* buf;
	int result;
	
	path = luaL_checkstring(L, 1);
	if (!lua_isuserdata(L, 2)) return luaL_typerror(L, 2, "userdata");
	buf = *(struct stat**)lua_touserdata(L, 2);
	
	result = lstat(path, buf);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__opendir(lua_State* L)
{
	const char* name;
	DIR* result;
	DIR** ud;
	
	name = luaL_checkstring(L, 1);
	
	result = opendir(name);
	if (!result)
	{
		lua_pushnil(L);
		return 1;
	}
	
	lua__push__DIR(L, result);
	return 1;
}

static int lua__readdir(lua_State* L)
{
	DIR* dir;
	struct dirent* entry;
	
	if (!lua_isuserdata(L, 1)) return luaL_typerror(L, 1, "userdata");
	dir = *(DIR**)lua_touserdata(L, 1);
	
	entry = readdir(dir);
	if (!entry)
	{
		int en = errno;
		lua_pushnil(L);
		return 1;
	}
	
	lua__push__dirent(L, entry);
	return 1;
}

static int lua__closedir(lua_State* L)
{
	DIR** pdir;
	int result;
	
	if (!lua_isuserdata(L, 1)) return luaL_typerror(L, 1, "userdata");
	pdir = (DIR**)lua_touserdata(L, 1);
	
	result = closedir(*pdir);
	*pdir = 0;
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__mkdir(lua_State* L)
{
	const char* pathname;
	mode_t mode = 0;
	int result;
	
	pathname = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);
	mode = lua__to__stat_mode(L, 2);
	
	result = mkdir(pathname, mode);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__rmdir(lua_State* L)
{
	const char* pathname;
	int result;
	
	pathname = luaL_checkstring(L, 1);
	
	result = rmdir(pathname);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__mknod(lua_State* L)
{
	const char* pathname;
	mode_t mode = 0;
	dev_t dev = 0;
	int result;
	
	pathname = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);
	mode = lua__to__stat_mode(L, 2);
	dev = luaL_checknumber(L, 3);
	
	result = mknod(pathname, mode, dev);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__unlink(lua_State* L)
{
	const char* pathname;
	int result;
	
	pathname = luaL_checkstring(L, 1);
	
	result = unlink(pathname);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__open(lua_State* L)
{
	const char* pathname;
	int flags;
	int result;
	
	pathname = luaL_checkstring(L, 1);
	luaL_checktype(L, 2, LUA_TTABLE);
	flags = lua__to__open_flags(L, 2);
	
	result = open(pathname, flags);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__close(lua_State* L)
{
	int fd;
	int result;
	
	fd = luaL_checknumber(L, 1);
	
	result = close(fd);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__read(lua_State* L)
{
	int fd;
	void* buf;
	size_t count;
	int result;
	
	fd = luaL_checknumber(L, 1);
	if (!lua_isuserdata(L, 2)) return luaL_typerror(L, 2, "userdata");
	buf = lua_touserdata(L, 2);
	count = luaL_checknumber(L, 3);
	
	result = read(fd, buf, count);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__write(lua_State* L)
{
	int fd;
	void* buf;
	size_t count;
	int result;
	
	fd = luaL_checknumber(L, 1);
	if (!lua_isuserdata(L, 2)) return luaL_typerror(L, 2, "userdata");
	buf = lua_touserdata(L, 2);
	count = luaL_checknumber(L, 3);
	
	result = write(fd, buf, count);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__lseek(lua_State* L)
{
	int fd;
	off_t offset;
	const char* strwhence;
	int whence;
	off_t result;
	
	fd = luaL_checknumber(L, 1);
	offset = luaL_checknumber(L, 2);
	if (!lua_isstring(L, 3)) return luaL_typerror(L, 3, "'SET', 'CUR' or 'END'");
	strwhence = lua_tostring(L, 3);
	if (!strcmp(strwhence, "SET")) whence = SEEK_SET;
	else if (!strcmp(strwhence, "CUR")) whence = SEEK_CUR;
	else if (!strcmp(strwhence, "END")) whence = SEEK_END;
	else return luaL_typerror(L, 3, "'SET', 'CUR' or 'END'");
	
	result = lseek(fd, offset, whence);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__statvfs(lua_State* L)
{
	const char* path;
	struct statvfs* buf;
	int result;
	
	path = luaL_checkstring(L, 1);
	if (!lua_isuserdata(L, 2)) return luaL_typerror(L, 2, "userdata");
	buf = *(struct statvfs**)lua_touserdata(L, 2);
	
	result = statvfs(path, buf);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__utimes(lua_State* L)
{
	const char* filename;
	struct timeval* times;
	int result;
	
	filename = luaL_checkstring(L, 1);
	if (!lua_isuserdata(L, 2)) return luaL_typerror(L, 2, "userdata");
	times = *(struct timeval**)lua_touserdata(L, 2);
	
	result = utimes(filename, times);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__rename(lua_State* L)
{
	const char* oldpath;
	const char* newpath;
	int result;
	
	oldpath = luaL_checkstring(L, 1);
	newpath = luaL_checkstring(L, 2);
	
	result = rename(oldpath, newpath);
	
	lua_pushnumber(L, result);
	return 1;
}

static int lua__getcwd(lua_State* L)
{
	char* buf;
	size_t size;
	char* result;
	
	if (!lua_isuserdata(L, 1)) return luaL_typerror(L, 1, "userdata");
	buf = lua_touserdata(L, 1);
	size = luaL_checknumber(L, 2);
	
	result = getcwd(buf, size);
	
	lua_pushlightuserdata(L, result);
	return 1;
}

/****************************************************************************/

static luaL_Reg functions[] = {
	{"new", lua__new},
	{"stat", lua__stat},
	{"fstat", lua__fstat},
	{"lstat", lua__lstat},
	{"opendir", lua__opendir},
	{"mkdir", lua__mkdir},
	{"rmdir", lua__rmdir},
	{"mknod", lua__mknod},
	{"unlink", lua__unlink},
	{"open", lua__open},
	{"close", lua__close},
	{"read", lua__read},
	{"write", lua__write},
	{"lseek", lua__lseek},
	{"statvfs", lua__statvfs},
	{"utimes", lua__utimes},
	{"rename", lua__rename},
	{"getcwd", lua__getcwd},
	{0, 0},
};

LUA_LIB int luaopen_module(lua_State* L)
{
	lua_newtable(L);
	luaL_register(L, 0, functions);
	lua_pushnumber(L, PATH_MAX);
	lua_setfield(L, -2, "PATH_MAX");
	return 1;
}

/*
Copyright (c) 2007 Jérôme Vuarand

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// vi: ts=4 sts=4 sw=4 encoding=utf-8

