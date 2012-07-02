#!/usr/bin/env lua
local fuse = require 'luse'
local udata = require 'luse.userdata'
local errno = require 'luse.errno'
local pio = require 'luse.posixio'


local red = "\27[31m"
local green = "\27[32m"
local yellow = "\27[33m"
local clean = "\27[0m"

local verbose = true

function info(msg)
    if verbose then
        print(green.."(!) "..msg..clean)
    end
end

function warning(msg)
    if verbose then
        print(yellow.."/!\\ "..msg..clean)
    end
end

function danger(msg)
    if verbose then
        print(red.."[!] "..msg..clean)
    end
end

local function mkset(array)
    local set = {}
    for _,flag in ipairs(array) do
        set[flag] = true
    end
    return set
end

local function splitpath(path)
    local elements = {}
    for element in path:gmatch("[^/]+") do 
        table.insert(elements, element)
    end
    return elements
end

local function mkset(array)
    local set = {}
    for _,flag in ipairs(array) do
        set[flag] = true
    end
    return set
end

local tags = {spam={}}
local filedb = {foo={spam=true}}

local function gettaggedas(taglist)
    local results = {}
    for file,tagset in pairs(filedb) do
        results[file] = true
        if taglist then
            for _,tag in ipairs(taglist) do
                if not tagset[tag] then
                    results[file] = false
                    break
                end
            end
        end
    end
    return results
end

function smartsplit(path)
    local tags = splitpath(path)
    last = tags[#tags]
    tags[#tags] = nil
    if #tags == 0 then tags = nil end
    return tags, last
end

local root = assert((...), "no root directory specified")

-- when the process is detached from the console, its directory changed, so we
-- make sure root is an absolute path
if root:sub(1,1)~='/' then
    local buf = udata.new(pio.PATH_MAX)
    assert(pio.getcwd(buf.data, buf.size)==buf.data)
    buf = tostring(buf)
    buf = buf:sub(1, buf:find('%z')-1)
    root = tostring(buf).."/"..root
    buf = nil
end

local fwfs = {}

local fields = {'dev', 'ino', 'mode', 'nlink', 'uid', 'gid', 'rdev', 'size', 'atime', 'mtime', 'ctime', 'blksize', 'blocks'}

function fwfs:getattr(path, st)
    info("getattr! path->"..path)
    local tagset,last = smartsplit(path)
    -- FIXME, the loop protection should be included here as well
    -- if the last tag is not in the global tagset
    if not tags[last] then
        -- it is probably a file
        local pst = pio.new 'stat'
        -- XXX FUGLY
        -- protection for the / dir
        last = last or ""
        -- so we look and see if we have such a file
        if pio.stat(root..'/'..last, pst)~=0 then
            return -errno.errno
        end
        for _,k in ipairs(fields) do
            st[k] = pst[k]
        end
    else
        st.mode = mkset({"IFDIR", "IRUSR", "IWUSR", "IXUSR", "IRGRP", "IWGRP", "IXGRP", "IROTH", "IWOTH", "IXOTH" })
        st.size = 4096
        st.nlink = 2
    end
end

local descriptors = {}

function fwfs:mkdir(path, mode)
    info("mkdir! path->"..path.." mode->"..tostring(mode))
    path = splitpath(path)
    -- XXX I wonder what happens if we don't check for presence
    for _,tag in ipairs(path) do
        if not tags[tag] then
            tags[tag] = {}
        end
    end
    --if pio.mkdir(root..path, mode)~=0 then
        --return -errno.errno
    --end
end

function fwfs:rmdir(path)
    info("rmdir! path->"..path)
    if pio.rmdir(root..path)~=0 then
        return -errno.errno
    end
end

function fwfs:opendir(path, fi)
    info("opendir! path->"..path.." fi->"..udata.tostring(fi))
    --local dir = pio.opendir(root..path)
    -- FIXME prevent loops here by refusing to open
    -- tags which contain themselves
    --if not dir then
        --return -errno.errno
    --end
    fi.fh = #descriptors+1
    -- XXX splitpath(path) is a bit arbitrary here
    -- maybe not even needed
    -- HA, IT IS NEEDED NOW
    descriptors[fi.fh] = {dir=splitpath(path)}
end

function fwfs:releasedir(path, fi)
    info("releasedir! path->"..path.." fi->"..udata.tostring(fi))
    if fi.fh~=0 then
        --descriptors[fi.fh].dir:closedir()
        descriptors[fi.fh] = nil
    else
        return -errno.EINVAL
    end
end

function fwfs:readdir(path, filler, offset, fi)
        -- filler: function value
        -- fi: userdata
    info("readdir! path->"..path.." offset->"..offset.." fi->"..udata.tostring(fi))
    if fi.fh~=0 then
        -- contains all tags (for now):
        for tag,_ in pairs(tags) do
            info("----------- entry->"..tag)
            filler(tag, nil, 0)
        end
        local dir = pio.opendir(root)
        local files = gettaggedas(descriptors[fi.fh].dir)
        repeat
            local entry = dir:readdir()
            if entry and files[entry.name] then
                info("----------- entry->"..entry.name)
                filler(entry.name, nil, 0)
           end
        until not entry
    end
end

function fwfs:mknod(path, mode, dev)
    info("mknod! path->"..path.." mode->"..tostring(mode).." dev->"..dev)
    if pio.mknod(root..path, mode, dev)~=0 then
        return -errno.errno
    end
end

function fwfs:unlink(path)
    info("unlink! path->"..path)
    if pio.unlink(root..path)~=0 then
        return -errno.errno
    end
end

function fwfs:open(path, fi)
    info("open! path->"..path.." fi->"..tostring(fi))
    -- XXX is this elegant?
    tagset,filename = smartsplit(path)
    if gettaggedas(tagset)[filename] then
        local fd = pio.open(root.."/"..filename, fi.flags)
        if fd==-1 then
            return -errno.errno
        end
        fi.fh = #descriptors+1
        descriptors[fi.fh] = {fd=fd, offset=0}
    else
        return -errno.ENOENT
    end
end


function fwfs:release(path, fi)
    info("release! path->"..path.." fi->"..tostring(fi))
    if fi.fh~=0 then
        pio.close(descriptors[fi.fh].fd)
        descriptors[fi.fh] = nil
    else
        return -errno.EINVAL
    end
end

function fwfs:read(path, buf, size, offset, fi)
    info("read: offset="..offset.." size="..size)
    if fi.fh~=0 then
        local descriptor = descriptors[fi.fh]
        if descriptor.offset~=offset then
            pio.lseek(descriptor.fd, offset, 'SET')
            descriptor.offset = offset
        end
        size = pio.read(descriptor.fd, buf, size)
        descriptor.offset = descriptor.offset + size
        return size
    else
        return -errno.EINVAL
    end
end

function fwfs:write(path, buf, size, offset, fi)
    info("write: offset="..offset.." size="..size)
    if fi.fh~=0 then
        local descriptor = descriptors[fi.fh]
        if descriptor.offset~=offset then
            warning("seeking")
            pio.lseek(descriptor.fd, offset, 'SET')
            descriptor.offset = offset
        end
        size = pio.write(descriptor.fd, buf, size)
        descriptor.offset = descriptor.offset + size
        return size
    else
        return -errno.EINVAL
    end
end

local fields = {'bsize', 'frsize', 'blocks', 'bfree', 'bavail', 'files', 'ffree', 'favail', 'fsid', 'flag', 'namemax'}
function fwfs:statfs(path, st)
    info("statfs! path->"..path.." st->"..tostring(st))
    local pst = pio.new 'statvfs'
    if pio.statvfs(root..path, pst)~=0 then
        return -errno.errno
    end
    local pbsize = pst.bsize
    for _,k in ipairs(fields) do
        st[k] = pst[k]
    end
end

function fwfs:utimens(path, time)
    info("utimens('"..path.."', {{sec="..time[1].sec..", nsec="..time[1].nsec.."}, {sec="..time[2].sec..", nsec="..time[2].nsec.."}})")
    local times = pio.new('timeval', 2)
    times[1].sec = time[1].sec
    times[1].usec = time[1].nsec / 1000
    times[2].sec = time[2].sec
    times[2].usec = time[2].nsec / 1000
    return pio.utimes(root..path, times)
end

function fwfs:rename(oldpath, newpath)
    info("rename! oldpath->"..oldpath.." newpath->"..newpath)
    return pio.rename(root..oldpath, root..newpath)
end

local args = {"fwfs", select(2, ...)}
for _,arg in ipairs(args) do
    if arg=='-d' then
        verbose = true
    end
end
fuse.main(args, fwfs)

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


