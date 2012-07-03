DESIGN
======


Backend
-----
- use one directory plus database file (valid .lua file or sqlite db) as backend
- all files are stored in this single directory, hence file names must be unique
- "file exists"-errors can be thrown for files that are somewhere in the fs but not visible in current tag path/space. (will break many shell scripts and other programs though)
- possible to map tagged structure to normal directory tree as well (maybe later? combinatorial explosion? hard links?)
- permissions don't change per tag space

Frontend
----
- all files are visible in the root
- (hard) links into their tag directories (are hard links hard? maybe later)
- subdirectory paths model the tag space
- all possible tags are visible in all directories (for now, possibly already excluding the tags already part of the path)
- addition of a file to a tag path tags it with all tags in the path, it is now accessible from any tag in the path. d'oh!
- deletion of a file in a tag path deletes all tags from said file
- deletion of a file from all tags it carries deletes it completely (optional: leave it in the backend for now, or tag it as "trash")
- the view (if at all easily possible) should keep the semantics of hard links to the root visible

Random Notes
------------
- getattr has a "stat" structure that it wants populated. funny way of doing things.
- let's have a slight combinatorial explosion by storing all possible subtags of each tag
- maybe we'll have to mount in 2 different modes. one for editing and one for adding. can't distinguish between stuff coming in and stuff being copied internally

First Steps
-----------
- start from the "fwfs" example of "luse" - DONE
- create dummy mkdir, creating volatile "directories" - DONE
- account for these in "opendir" - DONE (what about getattr?)
- create a function to filter path, to determine if a file in root is tagged this way - SEMI-DONE
- don't map directory creation to the origin root any more but keep in memory - DONE
- implement backend file"path" determination AND a way to split to the tag part - DONE
- implement smart mknod, checking if filename exists, verifying have to distinguish between new files in the fs and retagged ones - TODO
- implement rename with retagging semantics - MOSTLY-DONE
- implement rename for files - DONE
- implement remove for files with tagging semantics - DONE
- implement rmdir with tagging semantics - DONE
- introduce limbo for files being copied with already existing name in backed - TODO? - maybe stick to mv for now
- bytewise compare files from limbo that are written to with content the file with that name has in backend - TODO
- fix the bug that you can't put new files into the root - DONE
- fix ordering of readdir output to put tags first - TODO (is it possible?)
- find out how linking works - TODO
- make directories/tags persistent over remounts - DONE
- remove potential circularity of tags - DONE
- make document root readonly, only display files already tagged - TODO
- make removal of files not depend on currently invisible tags - TODO
- figure out what to do when renaming a tagpath to another one - TODO
- figure out how to treat files and tags with name clashes - DONE 
- figure out if you can mount fuse on top of the backend directory - TODO - most likely impossibru due to recursion
- find out if there is a way to abort mounting to print out warnings about mountpoint or backenddir - TODO
- maybe put all files not into root but into special tag 'ALL' including untagged ones - TODO
- add correct link counts in tags - TODO
- maybe make complete deletion of file in case it doesn't have any tags any more optional - TODO
- add configurable behavior - TODO
- rename this file to NOTES.md - TODO
- create proper README.md - TODO
- figure out what to do next - TODO


Desired Configurable Behavior
-----------------------------
- maybe as one option
    - tag removal only in the root (otherwise permission denied)
        - alternative: rmdir in tagpath removes all tags from path
    - complete file removal only in root (otherwise permission denied)
        - alternative: rm in tagpath removes also file if file has no more tags
- deleting a tag and reinstating it causes "undelete", yes/no?
    - (hint: to permanently delete all files form a tag do rm /tag/\*, or rm -rf /tag/\*)
- deleting a file that has tags in the root removes it, yes/no?
    - this clashes with "undelete" function above, at least for now




Corner Cases
------------
- what happens if in the backend directory there is a file which doesn't show up in the db? will it be invisible but impossible to create? will it be there but untagged? (maybe in an "untagged" dir?)


Questions
---------
- do dirs always have to be created manually(fuse limitation)? (I.e., does fuse do the same thing as plain "ls" to verify that dir is actually there? Or can we simply write to a non-existing dir and have our backend create it on the fly? (For now: manual creation)
    - getattr might be the problem
    - although ls does "opendir" and "readdir"

- how to implement hard links?

Future
------
- infer new tags without having to create them from the path that is explored by getattr (unlikely that this will work)
- for now, we ignore all files in subdirectories possibly present in the backend dir. maybe do something smart here?
- make undelete of directories/tags possible -> it's easy
- it is possible to only make tags deletable individualy (i.e., only in the root) -> is this a good idea maybe?
- allow name clashes in filenames, just don't display (or display with some numbered prefix) them unless the tagpath is unique
- write a visual frontend using link counts as hints for tagsize
