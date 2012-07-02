#!/usr/bin/env lua
local fuse = require 'luse'
local udata = require 'luse.userdata'
local errno = require 'luse.errno'

local msg = "Hello World!\n"

local function mkset(array)
	local set = {}
	for _,flag in ipairs(array) do
		set[flag] = true
	end
	return set
end

local hellofs = {}

function hellofs:getattr(path, st)
	if path=="/" then
		st.mode = mkset{ "IFDIR", "IRUSR", "IWUSR", "IXUSR", "IRGRP", "IXGRP", "IROTH", "IXOTH" }
		st.nlink = 2
	elseif path=="/hello" then
		st.mode = mkset{ "IFREG", "IRUSR", "IWUSR", "IRGRP", "IROTH" }
		st.nlink = 1
		st.size = #msg
	else
		return -errno.ENOENT
	end
end

function hellofs:readdir(path, filler, offset, fi)
	if path~="/" then
		return -errno.ENOENT
	end
	filler(".", nil, 0)
	filler("..", nil, 0)
	filler("hello", nil, 0)
end

function hellofs:open(path, fi)
	if path~="/hello" then
		return -errno.ENOENT
	end
	if fi.flags.O_WRONLY or fi.flags.O_RDWR then
		return -errno.EACCES	
	end
end

function hellofs:opendir(path, fi)
	if path~="/" then
		return -errno.ENOENT
	end
	if fi.flags.O_WRONLY or fi.flags.O_RDWR then
		return -errno.EACCES	
	end
end

function hellofs:read(path, buf, size, offset, fi)
	if path~="/hello" then
		return -errno.ENOENT
	end
	local len = #msg
	if offset<len then
		if offset + size > len then
			size = len - offset
		end
		udata.memcpy(buf, msg:sub(offset+1, offset+size), size)
	else
		size = 0
	end
	return size
end

local args = {"hellofs", ...}
fuse.main(args, hellofs)

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

