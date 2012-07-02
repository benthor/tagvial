#include <errno.h>
#include <string.h>
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

/****************************************************************************/

static int module__index(lua_State* L)
{
	if (lua_isstring(L, 2))
	{
		const char* k;
		k = lua_tostring(L, 2);
		if (!strcmp(k, "errno"))
			lua_pushnumber(L, errno);
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int lua__strerror(lua_State* L)
{
	int err;
	const char* result;
	
	err = luaL_checknumber(L, 1);
	
	result = strerror(err);
	
	lua_pushstring(L, result);
	return 1;
}

static const luaL_reg functions[] = {
	{"strerror", lua__strerror},
	{0, 0},
};

LUA_API int luaopen_module(lua_State *L)
{
	lua_newtable(L);
	luaL_register(L, 0, functions);
	#define REGISTER_ERROR(error) \
		lua_pushnumber(L, error); lua_setfield(L, -2, #error); \
		lua_pushstring(L, #error); lua_pushnumber(L, error); lua_settable(L, -3);
	REGISTER_ERROR(EPERM)
	REGISTER_ERROR(ENOENT)
	REGISTER_ERROR(ESRCH)
	REGISTER_ERROR(EINTR)
	REGISTER_ERROR(EIO)
	REGISTER_ERROR(ENXIO)
	REGISTER_ERROR(E2BIG)
	REGISTER_ERROR(ENOEXEC)
	REGISTER_ERROR(EBADF)
	REGISTER_ERROR(ECHILD)
	REGISTER_ERROR(EAGAIN)
	REGISTER_ERROR(ENOMEM)
	REGISTER_ERROR(EACCES)
	REGISTER_ERROR(EFAULT)
	REGISTER_ERROR(ENOTBLK)
	REGISTER_ERROR(EBUSY)
	REGISTER_ERROR(EEXIST)
	REGISTER_ERROR(EXDEV)
	REGISTER_ERROR(ENODEV)
	REGISTER_ERROR(ENOTDIR)
	REGISTER_ERROR(EISDIR)
	REGISTER_ERROR(EINVAL)
	REGISTER_ERROR(ENFILE)
	REGISTER_ERROR(EMFILE)
	REGISTER_ERROR(ENOTTY)
	REGISTER_ERROR(ETXTBSY)
	REGISTER_ERROR(EFBIG)
	REGISTER_ERROR(ENOSPC)
	REGISTER_ERROR(ESPIPE)
	REGISTER_ERROR(EROFS)
	REGISTER_ERROR(EMLINK)
	REGISTER_ERROR(EPIPE)
	REGISTER_ERROR(EDOM)
	REGISTER_ERROR(ERANGE)
	REGISTER_ERROR(ENOSYS)
	#undef REGISTER_ERROR
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, module__index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);
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

