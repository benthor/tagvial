#!/usr/bin/env lua
local fuse = require 'luse'
local udata = require 'luse.userdata'
local errno = require 'luse.errno'

local function mkset(array)
	local set = {}
	for _,flag in ipairs(array) do
		set[flag] = true
	end
	return set
end

local root = {}

local function splitpath(path)
	local elements = {}
	for element in path:gmatch("[^/]+") do
		table.insert(elements, element)
	end
	return elements
end

local function getfile(path)
	path = splitpath(path)
	local node = root
	local parent = nil
	for _,file in ipairs(path) do
		if type(node)~='table' then return nil end
		parent = node
		node = node[file]
		if not node then break end
	end
	return node,parent
end

local luafs = {}

function luafs:getattr(path, st)
	local file = getfile(path)
	if file then
		if type(file)=='table' then
			st.mode = mkset{ "IFDIR", "IRUSR", "IWUSR", "IXUSR", "IRGRP", "IWGRP", "IXGRP", "IROTH", "IWOTH", "IXOTH" }
			st.size = 4096
			-- link count is important for some apps like 'find'
			-- - one from the parent dir (:TODO: actually there should be one for each parent)
			-- - one from the current dir (.)
			-- - one from each subdir (..)
			st.nlink = 2
			for _,subfile in pairs(file) do
				if type(file)=='table' then
					st.nlink = st.nlink + 1
				end
			end
		elseif type(file)=='string' then
			st.mode = mkset{ "IFREG", "IRUSR", "IWUSR", "IRGRP", "IWGRP", "IROTH", "IWOTH" }
			st.size = #file
			st.nlink = 1
		elseif type(file)=='userdata' then
			st.mode = mkset{ "IFREG", "IRUSR", "IRGRP", "IROTH" }
		end
	else
		return -errno.ENOENT
	end
end

local descriptors = {}

function luafs:mkdir(path, mode)
	local file,parent = getfile(path)
	if file then
		return -errno.EEXIST
	end
	if not parent then
		return -errno.ENOENT
	end
	local tpath = splitpath(path)
	local filename = tpath[#tpath]
	if not filename then
		return -errno.EINVAL
	end
	if type(parent)~='table' then
		return -errno.ENOTDIR
	end
	local file = {}
	parent[filename] = file
end

function luafs:rmdir(path)
	local file,parent = getfile(path)
	if not file then
		return -errno.ENOENT
	end
	if type(file)~='table' then
		return -errno.ENOTDIR
	end
	local tpath = splitpath(path)
	local filename = tpath[#tpath]
	if not filename then
		return -errno.EINVAL
	end
	parent[filename] = nil
end

function luafs:opendir(path, fi)
	local file,parent = getfile(path)
	if not file then
		return -errno.ENOENT
	end
	if type(file)~='table' then
		return -errno.ENOTDIR
	end
	local tpath = splitpath(path)
	local filename = tpath[#tpath]
	if not filename and path~="/" then
		return -errno.EINVAL
	end
	fi.fh = #descriptors+1
	descriptors[fi.fh] = {path=path, parent=parent, filename=filename, file=file}
end

function luafs:releasedir(path, fi)
	if fi.fh~=0 then
		descriptors[fi.fh] = nil
	else
		return -errno.EINVAL
	end
end

function luafs:readdir(path, filler, offset, fi)
	if fi.fh~=0 then
		local file = descriptors[fi.fh].file
		filler(".", nil, 0)
		filler("..", nil, 0)
		for name in pairs(file) do
			filler(name, nil, 0)
		end
	end
end

function luafs:mknod(path, mode, dev)
	local file,parent = getfile(path)
	if file then
		return -errno.EEXIST
	end
	if not parent then
		return -errno.ENOENT
	end
	local tpath = splitpath(path)
	local filename = tpath[#tpath]
	if not filename then
		return -errno.EINVAL
	end
	if type(parent)~='table' then
		return -errno.ENOTDIR
	end
	local file = ""
	parent[filename] = file
end

function luafs:unlink(path)
	local file,parent = getfile(path)
	if not file then
		return -errno.ENOENT
	end
	if type(file)=='table' then
		return -errno.EISDIR
	end
	local tpath = splitpath(path)
	local filename = tpath[#tpath]
	if not filename then
		return -errno.EINVAL
	end
	parent[filename] = nil
end

function luafs:open(path, fi)
	local file,parent = getfile(path)
	if not file then
		return -errno.ENOENT
	end
	if type(file)=='table' then
		return -errno.EISDIR
	end
	local tpath = splitpath(path)
	local filename = tpath[#tpath]
	if not filename then
		return -errno.EINVAL
	end
	fi.fh = #descriptors+1
	descriptors[fi.fh] = {path=path, parent=parent, filename=filename, file=file}
end

function luafs:release(path, fi)
	if fi.fh~=0 then
		descriptors[fi.fh] = nil
	else
		return -errno.EINVAL
	end
end

function luafs:read(path, buf, size, offset, fi)
	if fi.fh~=0 then
		local file = descriptors[fi.fh].file
		if type(file)=='string' then
			local len = #file
			if offset<len then
				if offset + size > len then
					size = len - offset
				end
				udata.memcpy(buf, file:sub(offset+1, offset+size), size)
			else
				size = 0
			end
		else
			size = 0
		end
		return size
	else
		return -errno.EINVAL
	end
end

function luafs:write(path, buf, size, offset, fi)
	if fi.fh~=0 then
		local descriptor = descriptors[fi.fh]
		local file,filename,parent = descriptor.file,descriptor.filename,descriptor.parent
		local input = udata.new(size)
		udata.memcpy(input, buf, size)
		input = tostring(input)
		if type(file)=='string' then
			local len = #file
			if offset<len then
				if offset + size >= len then
					file = file:sub(1,offset)..input
				else
					file = file:sub(1,offset)..input..file:sub(offset+size)
				end
			else
				file = file..string.rep('\0', offset-len)..input
			end
			parent[filename] = file
			descriptor.file = file
		else
			size = 0
		end
		return size
	else
		return -errno.EINVAL
	end
end

-- utimens necessary for 'touch'
function luafs:utimens(path, time)
	print("utimens('"..path.."', {{sec="..time[1].sec..", nsec="..time[1].nsec.."}, {sec="..time[2].sec..", nsec="..time[2].nsec.."}}) not implemented")
end

local args = {"luafs", ...}
fuse.main(args, luafs)

--[[
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
]]

-- vi: ts=4 sts=4 sw=4 encoding=utf-8

