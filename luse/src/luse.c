#ifndef FUSE_USE_VERSION
#error FUSE_USE_VERSION is not defined.
#endif
#if FUSE_USE_VERSION > 26
#error Versions newer than FUSE 2.6 (26) are not yet supported (FUSE 2.7.0 use APIv26).
#endif
#if FUSE_USE_VERSION < 25
#error Versions older than FUSE 2.5 (25) are not yet supported.
#endif
	
#define _GNU_SOURCE 1
#define _FILE_OFFSET_BITS 64

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fuse.h>
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

#include "posix_structs.h"

/****************************************************************************/

typedef struct lua_fuse_state
{
	lua_State* L;
	int index;
} lua_fuse_state_t;

#define PROLOGUE(methodname) \
	int result; \
	struct fuse_context* context; \
	lua_fuse_state_t* state; \
	lua_State* L; \
	int top; \
	 \
	context = fuse_get_context(); \
	state = (lua_fuse_state_t*)context->private_data; \
	L = state->L; \
	 \
	lua_pushcfunction(L, lua__##methodname); \
	top = lua_gettop(L); \
	lua_rawgeti(L, LUA_REGISTRYINDEX, state->index);

#define EPILOGUE() \
	lua_call(L, lua_gettop(L)-top, 1); \
	 \
	result = lua_tonumber(L, -1); \
	lua_pop(L, 1); \
	 \
	return result;

#define DEFINE_STUB(methodname) \
static int lua__##methodname(lua_State* L) \
{ \
	int result; \
	int top; \
	/* Save stack size */\
	top = lua_gettop(L); \
	 \
	/* Get user method */ \
	lua_getfield(L, 1, #methodname); \
	if (!lua_isfunction(L, -1)) \
	{ \
		lua_pushnumber(L, -ENOSYS); \
		return 1; \
	} \
	/* Insert method before arguments */ \
	lua_insert(L, 1); \
	/* Install error handler */ \
	lua_getglobal(L, "debug"); \
	lua_getfield(L, -1, "traceback"); \
	lua_replace(L, -2); \
	lua_insert(L, 1); \
	/* Call user method */ \
	result = lua_pcall(L, top, LUA_MULTRET, 1); \
	if (result!=0) \
	{ \
		printf("%s\n", lua_tostring(L, -1)); \
		lua_pushnumber(L, -EIO); \
		return 1; \
	} \
	 \
	/* No return value means no error, just return 0 */ \
	if (lua_gettop(L) < 2) \
	{ \
		lua_pushnumber(L, 0); \
		return 1; \
	} \
	/* We expect a number */ \
	if (!lua_isnumber(L, 2)) \
	{ \
		lua_pushnumber(L, -EIO); \
		return 1; \
	} \
	lua_pushvalue(L, 2); \
	return 1; \
}

/****************************************************************************/

static int lua__fuse_file_info__index(lua_State* L)
{
	struct fuse_file_info** pfi;
	pfi = lua_touserdata(L, 1);
	if (*pfi)
	{
		struct fuse_file_info* fi = *pfi;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "flags")) lua__push__open_flags(L, fi->flags);
			else if (!strcmp(k, "writepage")) lua_pushnumber(L, fi->writepage);
			else if (!strcmp(k, "direct_io")) lua_pushboolean(L, fi->direct_io);
			else if (!strcmp(k, "keep_cache")) lua_pushboolean(L, fi->keep_cache);
			else if (!strcmp(k, "flush")) lua_pushboolean(L, fi->flush);
			else if (!strcmp(k, "fh")) lua_pushnumber(L, fi->fh);
			else if (!strcmp(k, "lock_owner")) lua_pushnumber(L, fi->lock_owner);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__fuse_file_info__newindex(lua_State* L)
{
	struct fuse_file_info** pfi;
	pfi = lua_touserdata(L, 1);
	if (*pfi)
	{
		struct fuse_file_info* fi = *pfi;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "flags"))
			{
				if (lua_istable(L, 3))
				{
					int flags = 0;
					lua_pushnil(L);
					while (lua_next(L, 3))
					{
						if (lua_type(L, -2)==LUA_TSTRING)
						{
							const char* k = lua_tostring(L, -2);
							if (!strcmp(k, "APPEND"))			{ if (lua_toboolean(L, -1)!=0) flags |= O_APPEND; }
							else if (!strcmp(k, "ASYNC"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_ASYNC; }
							else if (!strcmp(k, "CREAT"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_CREAT; }
							else if (!strcmp(k, "DIRECT"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_DIRECT; }
							else if (!strcmp(k, "DIRECTORY"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_DIRECTORY; }
							else if (!strcmp(k, "EXCL"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_EXCL; }
							else if (!strcmp(k, "LARGEFILE"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_LARGEFILE; }
							else if (!strcmp(k, "NOATIME"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_NOATIME; }
							else if (!strcmp(k, "NOCTTY"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_NOCTTY; }
							else if (!strcmp(k, "NOFOLLOW"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_NOFOLLOW; }
							else if (!strcmp(k, "NONBLOCK"))	{ if (lua_toboolean(L, -1)!=0) flags |= O_NONBLOCK; }
							else if (!strcmp(k, "NDELAY"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_NDELAY; }
							else if (!strcmp(k, "SYNC"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_SYNC; }
							else if (!strcmp(k, "TRUNC"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_TRUNC; }
							else if (!strcmp(k, "RDWR"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_RDWR; }
							else if (!strcmp(k, "RDONLY"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_RDONLY; }
							else if (!strcmp(k, "WRONLY"))		{ if (lua_toboolean(L, -1)!=0) flags |= O_WRONLY; }
						}
						lua_pop(L, 1);
					}
					fi->flags = flags;
				}
			}
			else if (!strcmp(k, "writepage")) { if (lua_isnumber(L, 3)) fi->writepage = lua_tonumber(L, 3); }
			else if (!strcmp(k, "direct_io")) fi->direct_io = lua_toboolean(L, 3);
			else if (!strcmp(k, "keep_cache")) fi->keep_cache = lua_toboolean(L, 3);
			else if (!strcmp(k, "flush")) fi->flush = lua_toboolean(L, 3);
			else if (!strcmp(k, "fh")) { if (lua_isnumber(L, 3)) fi->fh = lua_tonumber(L, 3); }
			else if (!strcmp(k, "lock_owner")) { if (lua_isnumber(L, 3)) fi->lock_owner = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static void lua__push__fuse_file_info(lua_State* L, struct fuse_file_info* fi)
{
	struct fuse_file_info** pfi;
	pfi = (struct fuse_file_info**)lua_newuserdata(L, sizeof(struct fuse_file_info*));
	*pfi = fi;
	lua_newtable(L);
	lua_pushcfunction(L, lua__fuse_file_info__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__fuse_file_info__newindex);
	lua_setfield(L, -2, "__newindex");
	lua_setmetatable(L, -2);
}

/****************************************************************************/

DEFINE_STUB(getattr)
static int fuse__getattr(const char* path, struct stat* st)
{
	PROLOGUE(getattr)
	lua_pushstring(L, path);
	lua__push__stat(L, st);
	EPILOGUE()
}

DEFINE_STUB(readlink)
static int fuse__readlink(const char* path, char* buf, size_t size)
{
	PROLOGUE(readlink)
	lua_pushstring(L, path);
	lua_pushlightuserdata(L, buf);
	lua_pushnumber(L, size);
	EPILOGUE()
}

DEFINE_STUB(mknod)
static int fuse__mknod(const char* path, mode_t mode, dev_t rdev)
{
	PROLOGUE(mknod)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	lua_pushnumber(L, rdev);
	EPILOGUE()
}

DEFINE_STUB(mkdir)
static int fuse__mkdir(const char* path, mode_t mode)
{
	PROLOGUE(mkdir)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	EPILOGUE()
}

DEFINE_STUB(unlink)
static int fuse__unlink(const char* path)
{
	PROLOGUE(unlink)
	lua_pushstring(L, path);
	EPILOGUE()
}

DEFINE_STUB(rmdir)
static int fuse__rmdir(const char* path)
{
	PROLOGUE(rmdir)
	lua_pushstring(L, path);
	EPILOGUE()
}

DEFINE_STUB(symlink)
static int fuse__symlink(const char* linkname, const char* path)
{
	PROLOGUE(symlink)
	lua_pushstring(L, linkname);
	lua_pushstring(L, path);
	EPILOGUE()
}

DEFINE_STUB(rename)
static int fuse__rename(const char* oldpath, const char* newpath)
{
	PROLOGUE(rename)
	lua_pushstring(L, oldpath);
	lua_pushstring(L, newpath);
	EPILOGUE()
}

DEFINE_STUB(link)
static int fuse__link(const char* oldpath, const char* newpath)
{
	PROLOGUE(link)
	lua_pushstring(L, oldpath);
	lua_pushstring(L, newpath);
	EPILOGUE()
}

DEFINE_STUB(chmod)
static int fuse__chmod(const char* path, mode_t mode)
{
	PROLOGUE(chmod)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	EPILOGUE()
}

DEFINE_STUB(chown)
static int fuse__chown(const char* path, uid_t uid, gid_t gid)
{
	PROLOGUE(chown)
	lua_pushstring(L, path);
	lua_pushnumber(L, uid);
	lua_pushnumber(L, gid);
	EPILOGUE()
}

DEFINE_STUB(truncate)
static int fuse__truncate(const char* path, off_t size)
{
	PROLOGUE(truncate)
	lua_pushstring(L, path);
	lua_pushnumber(L, size);
	EPILOGUE()
}

DEFINE_STUB(open)
static int fuse__open(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(open)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(read)
static int fuse__read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	PROLOGUE(read)
	lua_pushstring(L, path);
	lua_pushlightuserdata(L, buf);
	lua_pushnumber(L, size);
	lua_pushnumber(L, offset);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(write)
static int fuse__write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi)
{
	PROLOGUE(write)
	lua_pushstring(L, path);
	lua_pushlightuserdata(L, (char*)buf);
	lua_pushnumber(L, size);
	lua_pushnumber(L, offset);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(statfs)
static int fuse__statfs(const char* path, struct statvfs* st)
{
	PROLOGUE(statfs)
	lua_pushstring(L, path);
	lua__push__statvfs(L, st);
	EPILOGUE()
}

DEFINE_STUB(flush)
static int fuse__flush(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(flush)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(release)
static int fuse__release(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(release)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(fsync)
static int fuse__fsync(const char* path, int isdatasync, struct fuse_file_info* fi)
{
	PROLOGUE(fsync)
	lua_pushstring(L, path);
	lua_pushboolean(L, isdatasync!=0?1:0);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(setxattr)
static int fuse__setxattr(const char* path, const char* name, const char* value, size_t size, int flags)
{
	PROLOGUE(setxattr)
	lua_pushstring(L, path);
	lua_pushstring(L, name);
	lua_pushlightuserdata(L, (void*)value);
	lua_pushnumber(L, size);
	lua_pushnumber(L, flags);
	EPILOGUE()
}

DEFINE_STUB(getxattr)
static int fuse__getxattr(const char* path, const char* name, char* value, size_t size)
{
	PROLOGUE(getxattr)
	lua_pushstring(L, path);
	lua_pushstring(L, name);
	lua_pushlightuserdata(L, value);
	lua_pushnumber(L, size);
	EPILOGUE()
}

DEFINE_STUB(listxattr)
static int fuse__listxattr(const char* path, char* list, size_t size)
{
	PROLOGUE(listxattr)
	lua_pushstring(L, path);
	lua_pushlightuserdata(L, list);
	lua_pushnumber(L, size);
	EPILOGUE()
}

DEFINE_STUB(removexattr)
static int fuse__removexattr(const char* path, const char* name)
{
	PROLOGUE(removexattr)
	lua_pushstring(L, path);
	lua_pushstring(L, name);
	EPILOGUE()
}

DEFINE_STUB(opendir)
static int fuse__opendir(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(opendir)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

static int lua__fuse_fill_dir(lua_State* L)
{
	void* buf;
	fuse_fill_dir_t filler;
	buf = lua_touserdata(L, lua_upvalueindex(1));
	filler = lua_touserdata(L, lua_upvalueindex(2));
	luaL_checktype(L, 1, LUA_TSTRING);
	luaL_checktype(L, 2, LUA_TNIL);
	luaL_checktype(L, 3, LUA_TNUMBER);
	filler(buf, lua_tostring(L, 1), 0, lua_tonumber(L, 3));
	return 0;
}

DEFINE_STUB(readdir)
static int fuse__readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t off, struct fuse_file_info* fi)
{
	PROLOGUE(readdir)
	lua_pushstring(L, path);
	lua_pushlightuserdata(L, buf);
	lua_pushlightuserdata(L, filler);
	lua_pushcclosure(L, lua__fuse_fill_dir, 2);
	lua_pushnumber(L, off);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(releasedir)
static int fuse__releasedir(const char* path, struct fuse_file_info* fi)
{
	PROLOGUE(releasedir)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(fsyncdir)
static int fuse__fsyncdir(const char* path, int datasync, struct fuse_file_info* fi)
{
	PROLOGUE(fsyncdir)
	lua_pushstring(L, path);
	lua_pushnumber(L, datasync);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

/* intialize the state before calling fuse_main */
static lua_fuse_state_t* init__23__state = 0;
static void* fuse__init__23()
{
	lua_fuse_state_t* state = init__23__state;
	/* we reset global state pointer because we may have several ones */
	init__23__state = 0;
	return state;
}

static void* fuse__init(struct fuse_conn_info* conn)
{
	return 0;
}

static void fuse__destroy(void* data)
{
	(void)data;
}

DEFINE_STUB(access)
static int fuse__access(const char* path, int mask)
{
	PROLOGUE(access)
	lua_pushstring(L, path);
	lua_pushnumber(L, mask);
	EPILOGUE()
}

DEFINE_STUB(create)
static int fuse__create(const char* path, mode_t mode, struct fuse_file_info* fi)
{
	PROLOGUE(create)
	lua_pushstring(L, path);
	lua__push__stat_mode(L, mode);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(ftruncate)
static int fuse__ftruncate(const char* path, off_t size, struct fuse_file_info* fi)
{
	PROLOGUE(ftruncate)
	lua_pushstring(L, path);
	lua_pushnumber(L, size);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}

DEFINE_STUB(fgetattr)
static int fuse__fgetattr(const char* path, struct stat* st, struct fuse_file_info* fi)
{
	PROLOGUE(fgetattr)
	lua_pushstring(L, path);
	lua__push__stat(L, st);
	lua__push__fuse_file_info(L, fi);
	EPILOGUE()
}
/*
DEFINE_STUB(lock)
static int fuse__lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* lock)
{
	PROLOGUE(lock)
	lua_pushstring(L, path);
	lua__push__fuse_file_info(L, fi);
	lua_pushnumber(L, cmd);
	lua_pushlightuserdata(L, lock);
	EPILOGUE()
}
*/
DEFINE_STUB(utimens)
static int fuse__utimens(const char* path, const struct timespec tv[2])
{
	PROLOGUE(utimens)
	lua_pushstring(L, path);
	lua_createtable(L, 2, 0);
	lua_createtable(L, 0, 2);
	lua_pushnumber(L, tv[0].tv_sec); lua_setfield(L, -2, "sec");
	lua_pushnumber(L, tv[0].tv_nsec); lua_setfield(L, -2, "nsec");
	lua_rawseti(L, -2, 1);
	lua_createtable(L, 0, 2);
	lua_pushnumber(L, tv[1].tv_sec); lua_setfield(L, -2, "sec");
	lua_pushnumber(L, tv[1].tv_nsec); lua_setfield(L, -2, "nsec");
	lua_rawseti(L, -2, 2);
	EPILOGUE()
}

/****************************************************************************/

static int lua__main(lua_State *L)
{
	pthread_rwlock_t* lock;
	int largc, cargc;
	char** argv;
	size_t i;
	struct fuse_operations oper;
	
	luaL_checktype(L, 1, LUA_TTABLE);
	luaL_checktype(L, 2, LUA_TTABLE);
	
	largc = lua_objlen(L, 1);
	cargc = 1;
	argv = (char**)malloc((largc + cargc + 1) * sizeof(char*));
	for (i=0; i < largc; ++i)
	{
		lua_rawgeti(L, 1, i+1);
		if (lua_isstring(L, -1))
			argv[i] = strdup(lua_tostring(L, -1));
		else
			argv[i] = "";
		lua_pop(L, 1);
	}
	argv[i++] = "-s"; // Single threaded operation for the moment
	argv[i] = NULL;
	
	memset(&oper, 0, sizeof(struct fuse_operations));
	#define CHECK_METHOD(method) lua_getfield(L, 2, #method); if (lua_isfunction(L, -1)) oper.method = fuse__##method; lua_pop(L, 1);
	CHECK_METHOD(getattr)
	CHECK_METHOD(readlink)
//	CHECK_METHOD(getdir) /* deprecated */
	CHECK_METHOD(mknod)
	CHECK_METHOD(mkdir)
	CHECK_METHOD(unlink)
	CHECK_METHOD(rmdir)
	CHECK_METHOD(symlink)
	CHECK_METHOD(rename)
	CHECK_METHOD(link)
	CHECK_METHOD(chmod)
	CHECK_METHOD(chown)
	CHECK_METHOD(truncate)
//	CHECK_METHOD(utime) /* deprecated */
#if FUSE_USE_VERSION >= 22
	CHECK_METHOD(open)
	CHECK_METHOD(read)
	CHECK_METHOD(write)
#endif
#if FUSE_USE_VERSION >= 25
	CHECK_METHOD(statfs)
#endif
#if FUSE_USE_VERSION >= 22
	CHECK_METHOD(flush)
	CHECK_METHOD(release)
	CHECK_METHOD(fsync)
#endif
	CHECK_METHOD(setxattr)
	CHECK_METHOD(getxattr)
	CHECK_METHOD(listxattr)
	CHECK_METHOD(removexattr)
#if FUSE_USE_VERSION >= 23
	CHECK_METHOD(opendir)
	CHECK_METHOD(readdir)
	CHECK_METHOD(releasedir)
	CHECK_METHOD(fsyncdir)
#endif
#if FUSE_USE_VERSION >= 26
//	CHECK_METHOD(init) /* :TODO: */
#elif FUSE_USE_VERSION >= 23
	oper.init = fuse__init__23;
#endif
#if FUSE_USE_VERSION >= 23
//	CHECK_METHOD(destroy) /* :TODO: */
#endif
#if FUSE_USE_VERSION >= 25
	CHECK_METHOD(access)
	CHECK_METHOD(create)
	CHECK_METHOD(ftruncate)
	CHECK_METHOD(fgetattr)
#endif
#if FUSE_USE_VERSION >= 26
//	CHECK_METHOD(lock) /* :TODO: */
	CHECK_METHOD(utimens)
//	CHECK_METHOD(bmap) /* :TODO: */
#endif
	#undef CHECK_METHOD
	
	lua_fuse_state_t* state = (lua_fuse_state_t*)malloc(sizeof(lua_fuse_state_t));
	state->L = L;
	lua_pushvalue(L, 2);
	state->index = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_getfield(L, LUA_ENVIRONINDEX, "lock");
	lua_pop(L, 1);
	
#if FUSE_USE_VERSION >= 26
	fuse_main(largc + cargc, argv, &oper, state);
#elif FUSE_USE_VERSION >= 23
	if (init__23__state!=0) return luaL_error(L, "another FUSE filesystem is being run, try again later");
	init__23__state = state;
	fuse_main(largc + cargc, argv, &oper);
#endif
	
	luaL_unref(L, LUA_REGISTRYINDEX, state->index);
	free(state);
	state = 0;
	
	for (i=0; i<largc; ++i)
		free(argv[i]);
	free(argv);
	argv = 0;
	
	return 0;
}

static int lua__get_context(lua_State *L)
{
	struct fuse_context* context;
	context = fuse_get_context();
	if (lua_gettop(L) == 0)
		lua_createtable(L, 0, 3);
	else if (lua_type(L, 1)!=LUA_TTABLE)
		return luaL_typerror(L, 1, "table or nil");
	lua_pushnumber(L, context->uid); lua_setfield(L, -2, "uid");
	lua_pushnumber(L, context->gid); lua_setfield(L, -2, "gid");
	lua_pushnumber(L, context->pid); lua_setfield(L, -2, "pid");
	return 1;
}

/****************************************************************************/

static const luaL_reg functions[] = {
	{"main", lua__main},
	{"get_context", lua__get_context},
	{0, 0},
};

LUA_API int luaopen_module(lua_State *L)
{
	lua_newtable(L);
	luaL_register(L, 0, functions);
	lua_pushnumber(L, FUSE_USE_VERSION);
	lua_setfield(L, -2, "FUSE_USE_VERSION");
	lua_pushstring(L, LUSE_VERSION);
	lua_setfield(L, -2, "_VERSION");
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

