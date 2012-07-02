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

/****************************************************************************/

void lua__push__stat_mode(lua_State* L, mode_t m)
{
	lua_newtable(L);
	if (S_ISBLK(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IFBLK"); }
	if (S_ISCHR(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IFCHR"); }
	if (S_ISFIFO(m))	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IFIFO"); }
	if (S_ISREG(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IFREG"); }
	if (S_ISDIR(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IFDIR"); }
	if (S_ISLNK(m))		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IFLNK"); }
	if (S_ISSOCK(m))	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IFSOCK"); }
	if (m & S_IRUSR)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IRUSR"); }
	if (m & S_IWUSR)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IWUSR"); }
	if (m & S_IXUSR)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IXUSR"); }
	if (m & S_IRGRP)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IRGRP"); }
	if (m & S_IWGRP)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IWGRP"); }
	if (m & S_IXGRP)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IXGRP"); }
	if (m & S_IROTH)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IROTH"); }
	if (m & S_IWOTH)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IWOTH"); }
	if (m & S_IXOTH)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "IXOTH"); }
	if (m & S_ISUID)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "ISUID"); }
	if (m & S_ISGID)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "ISGID"); }
	if (m & S_ISVTX)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "ISVTX"); }
}

mode_t lua__to__stat_mode(lua_State* L, int index)
{
	mode_t m = 0;
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		if (lua_type(L, -2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, -2);
			if (!strcmp(k, "IFBLK"))		{ if (lua_toboolean(L, -1)!=0) m |= S_IFBLK; }
			else if (!strcmp(k, "IFBLK"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFBLK; }
			else if (!strcmp(k, "IFCHR"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFCHR; }
			else if (!strcmp(k, "IFIFO"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFIFO; }
			else if (!strcmp(k, "IFREG"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFREG; }
			else if (!strcmp(k, "IFDIR"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFDIR; }
			else if (!strcmp(k, "IFLNK"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFLNK; }
			else if (!strcmp(k, "IFSOCK"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IFSOCK; }
			else if (!strcmp(k, "IRUSR"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IRUSR; }
			else if (!strcmp(k, "IWUSR"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IWUSR; }
			else if (!strcmp(k, "IXUSR"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IXUSR; }
			else if (!strcmp(k, "IRGRP"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IRGRP; }
			else if (!strcmp(k, "IWGRP"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IWGRP; }
			else if (!strcmp(k, "IXGRP"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IXGRP; }
			else if (!strcmp(k, "IROTH"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IROTH; }
			else if (!strcmp(k, "IWOTH"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IWOTH; }
			else if (!strcmp(k, "IXOTH"))	{ if (lua_toboolean(L, -1)!=0) m |= S_IXOTH; }
			else if (!strcmp(k, "ISUID"))	{ if (lua_toboolean(L, -1)!=0) m |= S_ISUID; }
			else if (!strcmp(k, "ISGID"))	{ if (lua_toboolean(L, -1)!=0) m |= S_ISGID; }
			else if (!strcmp(k, "ISVTX"))	{ if (lua_toboolean(L, -1)!=0) m |= S_ISVTX; }
		}
		lua_pop(L, 1);
	}
	return m;
}

/****************************************************************************/

static int lua__stat__index(lua_State* L)
{
	struct stat** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct stat* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "dev")) lua_pushnumber(L, st->st_dev);
			else if (!strcmp(k, "ino")) lua_pushnumber(L, st->st_ino);
			else if (!strcmp(k, "mode")) lua__push__stat_mode(L, st->st_mode);
			else if (!strcmp(k, "nlink")) lua_pushnumber(L, st->st_nlink);
			else if (!strcmp(k, "uid")) lua_pushnumber(L, st->st_uid);
			else if (!strcmp(k, "gid")) lua_pushnumber(L, st->st_gid);
			else if (!strcmp(k, "rdev")) lua_pushnumber(L, st->st_rdev);
			else if (!strcmp(k, "size")) lua_pushnumber(L, st->st_size);
			else if (!strcmp(k, "atime")) lua_pushnumber(L, st->st_atime);
			else if (!strcmp(k, "mtime")) lua_pushnumber(L, st->st_mtime);
			else if (!strcmp(k, "ctime")) lua_pushnumber(L, st->st_ctime);
			else if (!strcmp(k, "blksize")) lua_pushnumber(L, st->st_blksize);
			else if (!strcmp(k, "blocks")) lua_pushnumber(L, st->st_blocks);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__stat__newindex(lua_State* L)
{
	struct stat** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct stat* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "dev")) { if (lua_isnumber(L, 3)) st->st_dev = lua_tonumber(L, 3); }
			else if (!strcmp(k, "ino")) { if (lua_isnumber(L, 3)) st->st_ino = lua_tonumber(L, 3); }
			else if (!strcmp(k, "mode")) { if (lua_istable(L, 3)) st->st_mode = lua__to__stat_mode(L, 3); }
			else if (!strcmp(k, "nlink")) { if (lua_isnumber(L, 3)) st->st_nlink = lua_tonumber(L, 3); }
			else if (!strcmp(k, "uid")) { if (lua_isnumber(L, 3)) st->st_uid = lua_tonumber(L, 3); }
			else if (!strcmp(k, "gid")) { if (lua_isnumber(L, 3)) st->st_gid = lua_tonumber(L, 3); }
			else if (!strcmp(k, "rdev")) { if (lua_isnumber(L, 3)) st->st_rdev = lua_tonumber(L, 3); }
			else if (!strcmp(k, "size")) { if (lua_isnumber(L, 3)) st->st_size = lua_tonumber(L, 3); }
			else if (!strcmp(k, "atime")) { if (lua_isnumber(L, 3)) st->st_atime = lua_tonumber(L, 3); }
			else if (!strcmp(k, "mtime")) { if (lua_isnumber(L, 3)) st->st_mtime = lua_tonumber(L, 3); }
			else if (!strcmp(k, "ctime")) { if (lua_isnumber(L, 3)) st->st_ctime = lua_tonumber(L, 3); }
			else if (!strcmp(k, "blksize")) { if (lua_isnumber(L, 3)) st->st_blksize = lua_tonumber(L, 3); }
			else if (!strcmp(k, "blocks")) { if (lua_isnumber(L, 3)) st->st_blocks = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static int lua__stat__gc(lua_State* L)
{
	struct stat** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		free(*pst);
		*pst = 0;
	}
}

// See http://www.opengroup.org/onlinepubs/009695399/basedefs/sys/stat.h.html
void lua__push__stat(lua_State* L, struct stat* st)
{
	struct stat** pst;
	pst = (struct stat**)lua_newuserdata(L, sizeof(struct stat*));
	if (!st)
	{
		*pst = (struct stat*)malloc(sizeof(struct stat));
		memset(*pst, 0, sizeof(struct stat));
	}
	else
		*pst = st;
	lua_newtable(L);
	lua_pushcfunction(L, lua__stat__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__stat__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!st)
	{
		lua_pushcfunction(L, lua__stat__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

/****************************************************************************/

void lua__push__statvfs_flag(lua_State* L, int flags)
{
	lua_newtable(L);
	if (flags & ST_RDONLY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "RDONLY"); }
	if (flags & ST_NOSUID)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "NOSUID"); }
}

int lua__to__statvfs_flag(lua_State* L, int index)
{
	int flags = 0;
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		if (lua_type(L, -2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, -2);
			if (!strcmp(k, "RDONLY"))		{ if (lua_toboolean(L, -1)!=0) flags |= ST_RDONLY; }
			else if (!strcmp(k, "NOSUID"))	{ if (lua_toboolean(L, -1)!=0) flags |= ST_NOSUID; }
		}
		lua_pop(L, 1);
	}
	return flags;
}

/****************************************************************************/

static int lua__statvfs__index(lua_State* L)
{
	struct statvfs** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct statvfs* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "bsize")) lua_pushnumber(L, st->f_bsize);
			else if (!strcmp(k, "frsize")) lua_pushnumber(L, st->f_frsize);
			else if (!strcmp(k, "blocks")) lua_pushnumber(L, st->f_blocks);
			else if (!strcmp(k, "bfree")) lua_pushnumber(L, st->f_bfree);
			else if (!strcmp(k, "bavail")) lua_pushnumber(L, st->f_bavail);
			else if (!strcmp(k, "files")) lua_pushnumber(L, st->f_files);
			else if (!strcmp(k, "ffree")) lua_pushnumber(L, st->f_ffree);
			else if (!strcmp(k, "favail")) lua_pushnumber(L, st->f_favail);
			else if (!strcmp(k, "fsid")) lua_pushnumber(L, st->f_fsid);
			else if (!strcmp(k, "flag")) lua__push__statvfs_flag(L, st->f_flag);
			else if (!strcmp(k, "namemax")) lua_pushnumber(L, st->f_namemax);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__statvfs__newindex(lua_State* L)
{
	struct statvfs** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		struct statvfs* st = *pst;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "bsize")) { if (lua_isnumber(L, 3)) st->f_bsize = lua_tonumber(L, 3); }
			else if (!strcmp(k, "frsize")) { if (lua_isnumber(L, 3)) st->f_frsize = lua_tonumber(L, 3); }
			else if (!strcmp(k, "blocks")) { if (lua_isnumber(L, 3)) st->f_blocks = lua_tonumber(L, 3); }
			else if (!strcmp(k, "bfree")) { if (lua_isnumber(L, 3)) st->f_bfree = lua_tonumber(L, 3); }
			else if (!strcmp(k, "bavail")) { if (lua_isnumber(L, 3)) st->f_bavail = lua_tonumber(L, 3); }
			else if (!strcmp(k, "files")) { if (lua_isnumber(L, 3)) st->f_files = lua_tonumber(L, 3); }
			else if (!strcmp(k, "ffree")) { if (lua_isnumber(L, 3)) st->f_ffree = lua_tonumber(L, 3); }
			else if (!strcmp(k, "favail")) { if (lua_isnumber(L, 3)) st->f_favail = lua_tonumber(L, 3); }
			else if (!strcmp(k, "fsid")) { if (lua_isnumber(L, 3)) st->f_fsid = lua_tonumber(L, 3); }
			else if (!strcmp(k, "flag")) { if (lua_istable(L, 3)) st->f_flag = lua__to__statvfs_flag(L, 3); }
			else if (!strcmp(k, "namemax")) { if (lua_isnumber(L, 3)) st->f_namemax = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static int lua__statvfs__gc(lua_State* L)
{
	struct statvfs** pst;
	pst = lua_touserdata(L, 1);
	if (*pst)
	{
		free(*pst);
		*pst = 0;
	}
}

// See http://www.opengroup.org/onlinepubs/009695399/basedefs/sys/statvfs.h.html
void lua__push__statvfs(lua_State* L, struct statvfs* st)
{
	struct statvfs** pst;
	pst = (struct statvfs**)lua_newuserdata(L, sizeof(struct statvfs*));
	if (!st)
	{
		*pst = (struct statvfs*)malloc(sizeof(struct statvfs));
		memset(*pst, 0, sizeof(struct statvfs));
	}
	else
		*pst = st;
	lua_newtable(L);
	lua_pushcfunction(L, lua__statvfs__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__statvfs__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!st)
	{
		lua_pushcfunction(L, lua__statvfs__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

/****************************************************************************/

void lua__push__open_flags(lua_State* L, int f)
{
	lua_newtable(L);
	if (f & O_APPEND)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "APPEND"); }
	if (f & O_ASYNC)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "ASYNC"); }
	if (f & O_CREAT)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "CREAT"); }
	if (f & O_DIRECT)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "DIRECT"); }
	if (f & O_DIRECTORY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "DIRECTORY"); }
	if (f & O_EXCL)			{ lua_pushboolean(L, 1); lua_setfield(L, -2, "EXCL"); }
	if (f & O_LARGEFILE)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "LARGEFILE"); }
	if (f & O_NOATIME)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "NOATIME"); }
	if (f & O_NOCTTY)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "NOCTTY"); }
	if (f & O_NOFOLLOW)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "NOFOLLOW"); }
	if (f & O_NONBLOCK)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "NONBLOCK"); }
	if (f & O_NDELAY)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "NDELAY"); }
	if (f & O_SYNC)			{ lua_pushboolean(L, 1); lua_setfield(L, -2, "SYNC"); }
	if (f & O_TRUNC)		{ lua_pushboolean(L, 1); lua_setfield(L, -2, "TRUNC"); }
	if (f & O_RDWR)			{ lua_pushboolean(L, 1); lua_setfield(L, -2, "RDWR"); }
	else if (f & O_RDONLY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "RDONLY"); }
	else if (f & O_WRONLY)	{ lua_pushboolean(L, 1); lua_setfield(L, -2, "WRONLY"); }
}

int lua__to__open_flags(lua_State* L, int index)
{
	int flags = 0;
	lua_pushnil(L);
	while (lua_next(L, index))
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
	return flags;
}

/****************************************************************************/

static int lua__dirent__index(lua_State* L)
{
	struct dirent** pent;
	pent = lua_touserdata(L, 1);
	if (*pent)
	{
		struct dirent* ent = *pent;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "ino")) lua_pushnumber(L, ent->d_ino);
			else if (!strcmp(k, "off")) lua_pushnumber(L, ent->d_off);
			else if (!strcmp(k, "reclen")) lua_pushnumber(L, ent->d_reclen);
			else if (!strcmp(k, "type")) lua_pushnumber(L, ent->d_type);
			else if (!strcmp(k, "name")) lua_pushstring(L, ent->d_name);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static inline int min(int a, int b)
{
	return a<b?a:b;
}

static int lua__dirent__newindex(lua_State* L)
{
	struct dirent** pent;
	pent = (struct dirent**)lua_touserdata(L, 1);
	if (*pent)
	{
		struct dirent* ent = *pent;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "ino")) { if (lua_isnumber(L, 3)) ent->d_ino = lua_tonumber(L, 3); }
			else if (!strcmp(k, "off")) { if (lua_isnumber(L, 3)) ent->d_off = lua_tonumber(L, 3); }
			else if (!strcmp(k, "reclen")) { if (lua_isnumber(L, 3)) ent->d_reclen = lua_tonumber(L, 3); }
			else if (!strcmp(k, "type")) { if (lua_isnumber(L, 3)) ent->d_reclen = lua_tonumber(L, 3); }
			else if (!strcmp(k, "name")) { if (lua_isstring(L, 3)) strncpy(ent->d_name, lua_tostring(L, 3), min(NAME_MAX, lua_objlen(L, 3))+1); }
		}
	}
	return 0;
}

static int lua__dirent__gc(lua_State* L)
{
	struct dirent** pent;
	pent = lua_touserdata(L, 1);
	if (*pent)
	{
		free(*pent);
		*pent = 0;
	}
}

void lua__push__dirent(lua_State* L, struct dirent* ent)
{
	struct dirent** pent;
	pent = (struct dirent**)lua_newuserdata(L, sizeof(struct dirent*));
	if (!ent)
	{
		*pent = (struct dirent*)malloc(sizeof(struct dirent));
		memset(*pent, 0, sizeof(struct dirent));
	}
	else
		*pent = ent;
	lua_newtable(L);
	lua_pushcfunction(L, lua__dirent__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__dirent__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!ent)
	{
		lua_pushcfunction(L, lua__dirent__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

/****************************************************************************/

static int lua__timeval__index(lua_State* L)
{
	struct timeval** ptime;
	ptime = lua_touserdata(L, 1);
	if (*ptime)
	{
		struct timeval* time = *ptime;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "sec")) lua_pushnumber(L, time->tv_sec);
			else if (!strcmp(k, "usec")) lua_pushnumber(L, time->tv_usec);
			else lua_pushnil(L);
			return 1;
		}
	}
	lua_pushnil(L);
	return 1;
}

static int lua__timeval__newindex(lua_State* L)
{
	struct timeval** ptime;
	ptime = lua_touserdata(L, 1);
	if (*ptime)
	{
		struct timeval* time = *ptime;
		if (lua_type(L, 2)==LUA_TSTRING)
		{
			const char* k = lua_tostring(L, 2);
			if (!strcmp(k, "sec")) { if (lua_isnumber(L, 3)) time->tv_sec = lua_tonumber(L, 3); }
			else if (!strcmp(k, "usec")) { if (lua_isnumber(L, 3)) time->tv_usec = lua_tonumber(L, 3); }
		}
	}
	return 0;
}

static int lua__timeval__gc(lua_State* L)
{
	struct timeval** ptime;
	ptime = lua_touserdata(L, 1);
	if (*ptime)
	{
		free(*ptime);
		*ptime = 0;
	}
}

void lua__push__timeval(lua_State* L, struct timeval* time)
{
	struct timeval** ptime;
	ptime = (struct timeval**)lua_newuserdata(L, sizeof(struct timeval*));
	if (!time)
	{
		*ptime = (struct timeval*)malloc(sizeof(struct timeval));
		memset(*ptime, 0, sizeof(struct statvfs));
	}
	else
		*ptime = time;
	lua_newtable(L);
	lua_pushcfunction(L, lua__timeval__index);
	lua_setfield(L, -2, "__index");
	lua_pushcfunction(L, lua__timeval__newindex);
	lua_setfield(L, -2, "__newindex");
	if (!time)
	{
		lua_pushcfunction(L, lua__timeval__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
}

static int lua__timeval_array__index(lua_State* L)
{
	if (lua_isnumber(L, 2))
	{
		size_t size;
		size_t index;
		
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, "size");
		size = lua_tonumber(L, -1);
		lua_pop(L, 2);
		index = lua_tonumber(L, 2) - 1;
		
		if (index>=0 && index<size)
		{
			struct timeval* times;
			times = *(struct timeval**)lua_touserdata(L, 1);
			lua__push__timeval(L, times+index);
			lua_createtable(L, 0, 1);
			lua_pushvalue(L, 1);
			lua_setfield(L, -2, "array");
			lua_setfenv(L, -2);
			return 1;
		}
		else
			lua_pushnil(L);
	}
	else
		lua_pushnil(L);
	return 1;
}

void lua__push__timeval_array(lua_State* L, struct timeval* times, size_t size)
{
	struct timeval** ptimes;
	ptimes = (struct timeval**)lua_newuserdata(L, sizeof(struct timeval*));
	if (!times)
	{
		*ptimes = (struct timeval*)malloc(sizeof(struct timeval) * size);
		memset(*ptimes, 0, sizeof(struct timeval) * size);
	}
	else
		*ptimes = times;
	lua_newtable(L);
	lua_pushnumber(L, size);
	lua_setfield(L, -2, "size");
	lua_pushcfunction(L, lua__timeval_array__index);
	lua_setfield(L, -2, "__index");
	if (!times)
	{
		lua_pushcfunction(L, lua__timeval__gc);
		lua_setfield(L, -2, "__gc");
	}
	lua_setmetatable(L, -2);
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

