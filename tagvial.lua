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
        print(yellow.."/!\\ ".. msg ..clean)
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

require 'persistence'

local dbname = '.db.lua'
-- local dbname = '.tagvialdb.lua'

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

local tags, filedb = {}, {}

local OPTIONS = {
    "noundelete",  -- mkdir'ing an rmdir'd tag doesn't retag its previous members
    "recursivermdir", -- deletes the entire tagpath not just the last tag
    "deletetagless", -- deletes files when all their tags are gone
    "allowrootrm", -- allows files to be deleted from the root
    }

local function savedb()
    persistence.store(root .. '/' .. dbname, tags, filedb)
end

local f = io.open(root .. '/' .. dbname, 'r')
if f~= nil then 
    io.close(f)
    tags,filedb = persistence.load(root ..'/'.. dbname)
else
    warning("database not found, creating a new one")
end

local function gettaggedas(taglist)
    local results = {}
    for file,tagset in pairs(filedb) do
        -- if no taglist given, all files are visible
        results[file] = true
        for _,tag in ipairs(taglist or {}) do
            if not tagset[tag] then
                results[file] = false
                break
            end
        end
    end
    return results
end

local function addtags(taglist, filename)
    local entry = filedb[filename] or {}
    -- be resilient against nil taglists
    for _,tag in ipairs(taglist or {}) do
        if not tags[tag] then tags[tag] = {} end
        entry[tag] = true
    end
    filedb[filename] = entry
    savedb()
end

local function smartsplit(path)
    local tags = splitpath(path)
    last = tags[#tags]
    tags[#tags] = nil
    if #tags == 0 then tags = nil end
    return tags, last
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
        -- make sure that we don't consider files not in the current tagset
        if last and not gettaggedas(tagset)[last] then
            return -errno.ENOENT
        end
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
        -- if we have a filename of this name already
        if filedb[tag] then
            return -errno.EEXIST
        end
        if not tags[tag] then
            tags[tag] = {}
        end
    end
    -- write to db
    savedb()
    --if pio.mkdir(root..path, mode)~=0 then
        --return -errno.errno
    --end
end

function fwfs:rmdir(path)
    info("rmdir! path->"..path)
    taglist = splitpath(path)
    if not locals['recursivermdir'] then
        taglist = {taglist[#taglist]}
    else
        warning("recursively deleting the entire path due to mount option")
    end
    for i,tag in ipairs(taglist) do
        warning("deleting "..tag)
        tags[tag] = nil
        if locals['noundelete'] then
            warning("permanently deleting " .. tag .. " due to disabled undelete")
            for file,tagset in pairs(filedb) do
                -- XXX put "nil" here instead of "false"
                -- otherwise the fact that a certain tag was present "leaks"
                -- OTOH, this facilitates implementation of an undelete tool ;)
                tagset[tag] = nil
            end
        end
    end
    savedb()
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
        -- remove those tags already in the tagpath
        local pathtags = mkset(splitpath(path))
        for tag,_ in pairs(tags) do
            if not pathtags[tag] then
                info("----------- entry->"..tag)
                filler(tag, nil, 0)
            end
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
    local taglist,filename = smartsplit(path)
    -- this currently prevents copying an already tagged file to another tag path XXX -> figure out if we can do "fwfs:create" and simply compare blocks with data to write and already existing file. if not equal, throw error
    if pio.mknod(root.."/"..filename, mode, dev)~=0 then
        return -errno.errno
    end
    addtags(taglist,filename)
end

function fwfs:unlink(path)
    info("unlink! path->"..path)
    local taglist, filename = smartsplit(path)
    local entry = filedb[filename]

    -- in case of trying to delete nonexisting entry
    -- (also prevents from accidentally deleting the database file)
    if not entry then
        return -errno.ENOENT
    end

    -- in case we try to delete from the root
    if not taglist then
        if not locals['allowrootrm'] then
            return -errno.EACCES
        else
            if pio.unlink(root..'/'..filename)~=0 then
                return -errno.errno
            else
                filedb[filename] = nil
                return
            end
        end
    end

    -- if we get here, file hasn't been deleted yet
    -- remove tags one by one
    for _,tag in ipairs(taglist) do
        entry[tag] = false
    end

    -- check if file has any tags left
    for _, value in pairs(entry) do
        if value then
            -- we found a still existing tag
            filedb[filename] = entry
            return
        end
    end

    -- if we get to this point, the file has no tags any more and is to be deleted
    if pio.unlink(root..'/'..filename)~=0 then
        return -errno.errno
    else
        filedb[filename] = nil
    end
end

function fwfs:open(path, fi)
    info("open! path->"..path.." fi->"..tostring(fi))
    -- XXX is this elegant?
    local taglist,filename = smartsplit(path)
    if gettaggedas(taglist)[filename] then
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
    local oldtags,oldlast = smartsplit(oldpath)
    local newtags,newlast = smartsplit(newpath)
    -- check if source and destination are both files
    if filedb[oldlast] then
        if filedb[newlast] then
            -- if filenames are equal
            if oldlast == newlast then
                -- we only need to retag
                local tagset = filedb[oldlast]
                for _,otag in ipairs(oldtags or {}) do
                    tagset[otag] = false
                end
                -- XXX maybe use addtags method here
                for _,ntag in ipairs(newtags or {}) do
                    tagset[ntag] = true
                end
                -- equivalent to filedb[oldlast] = tagset but hey
                filedb[newlast] = tagset
                savedb()
                
            else
                -- don't permit overwriting of existing files
                return -errno.EEXIST
            end
        else
            -- if the target filename is nonexisting
            -- we do a true renaming. discard all old tags and add the new
            filedb[oldlast] = nil
            filedb[newlast] = mkset(newtags or {})
            -- TODO, don't save the new state when pio.rename fails
            savedb()
            return pio.rename(root.."/"..oldlast, root.."/"..newlast)
        end
    else
        -- don't permit renaming of tag paths for now
        return -errno.EACCES
    end
end

-- filters argslist for certain options, passes on the rest
-- returns table of all filtered options set to true
local function filteroptions(argslist, filteropts)
    local options = ""
    local last = 0
    local strippedargs = {}

    for i,arg in ipairs(argslist) do
        arg,_ = arg:gsub("%s", "")
        if arg:sub(1,2) == '-o' then
            -- put everything we had after last option in arguments list
            for j=last,i-1 do
                table.insert(strippedargs, argslist[j])
            end
            local opt = arg:sub(3)
            local comma = false
            if opt ~= "" then
                options = options .. " " .. opt
            else
                -- HACK!
                comma = true
            end
            if opt:sub(-1) == ',' then
                comma = true
            end
            for j=i+1,#argslist do
                local opt = argslist[j]:gsub("%s", "")
                if not comma and opt:sub(1,1) ~= ',' then
                    break
                end
                if opt:sub(1,1) == "-" then
                    break
                end
                if opt:sub(-1) ~= "," then
                    comma = false
                end
                options = options .. " " .. opt
                last = j+1
            end
        end
    end
    if last == 0 then
        return argslist, {}
    end
    for i=last,#argslist do
        table.insert(strippedargs,argslist[i])
    end
    local localopts = {}
    local filteropts = mkset(filteropts)
    local minuso = false
    for opt in options:gmatch("[^,%s]+") do
        if filteropts[opt] then
            localopts[opt] = true
        else
            if not minuso then
                table.insert(strippedargs, "-o")
                minuso = true
            end
            table.insert(strippedargs, opt)
        end
    end
    return strippedargs, localopts
end


local args = {"tagvial", select(2, ...)}
args, locals = filteroptions(args, OPTIONS)
for _,arg in ipairs(args) do
    info(arg)
    if arg=='-d' then
        verbose = true
    end
end
fuse.main(args, fwfs)

--[[
Copyright (c) 2012 Thorben Krüger
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


