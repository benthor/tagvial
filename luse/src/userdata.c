#include <stdlib.h>
#include <string.h>
#define LUA_LIB
#include <lua.h>
#include <lauxlib.h>

/****************************************************************************/

static int lua__memcpy(lua_State* L)
{
	unsigned char* buf;
	size_t size;
	if (!lua_isuserdata(L, 1))
		return luaL_typerror(L, 1, "userdata");
	buf = lua_touserdata(L, 1);
	size = luaL_checknumber(L, 3);
	if (lua_type(L, 2)==LUA_TSTRING)
		memcpy(buf, lua_tostring(L, 2), size);
	else if (lua_isuserdata(L, 2))
		memcpy(buf, lua_touserdata(L, 2), size);
	return 0;
}

static int lua__userdata__tostring(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	lua_pushlstring(L, lua_touserdata(L, 1), lua_objlen(L, 1));
	return 1;
}

static int lua__userdata__index(lua_State* L)
{
	if (lua_type(L, 2)==LUA_TSTRING)
	{
		const char* k = lua_tostring(L, 2);
		if (!strcmp(k, "size"))
			lua_pushnumber(L, lua_objlen(L, 1));
		else if (!strcmp(k, "data"))
			lua_pushlightuserdata(L, lua_touserdata(L, 1));
		else
			lua_pushnil(L);
	}
	else if (lua_isnumber(L, 2))
	{
		ptrdiff_t offset;
		offset = lua_tonumber(L, 2);
		if (offset >= 0 && offset < lua_objlen(L, 1))
			lua_pushlightuserdata(L, ((char*)lua_touserdata(L, 1)) + offset);
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L);
	return 1;
}

static int lua__new(lua_State* L)
{
	size_t size;
	size = luaL_checknumber(L, 1);
	lua_newuserdata(L, size);
	lua_createtable(L, 0, 1);
	lua_pushcfunction(L, lua__userdata__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__userdata__tostring);
	lua_setfield(L, -2, "__tostring");
	lua_setmetatable(L, -2);
	return 1;
}

/****************************************************************************/

static const luaL_reg functions[] = {
	{"memcpy", lua__memcpy},
	{"new", lua__new},
	{"tostring", lua__userdata__tostring},
	{0, 0},
};

LUA_API int luaopen_module(lua_State *L)
{
	lua_newtable(L);
	luaL_register(L, 0, functions);
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

