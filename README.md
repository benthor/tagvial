DESIGN
======

Notes
-----
I _know_ MVC is riddiculously overhyped. I am merely attempting to find some merits in it by specifying my toy filesystem adhering to the concept. I basically want to see what it's all about. 


Model
-----
- use one directory plus database file (valid .lua file or sqlite db) as backend
- all files are stored in this single directory, hence file names must be unique
- "file exists"-errors can be thrown for files that are somewhere in the fs but not visible in current tag path/space. (will break many shell scripts and other programs though)
- possible to map tagged structure to normal directory tree as well (maybe later? combinatorial explosion? hard links?)
- permissions don't change per tag space

View
----
- all files are visible in the root
- (hard) links into their tag directories
- subdirectory paths model the tag space
- all possible tags are visible in all directories (for now, possibly already excluding the tags already part of the path)
- addition of a file to a tag path tags it with all tags in the path, it is now accessible from any tag in the path. d'oh!
- deletion of a file in a tag path deletes all tags from said file
- deletion of a file from all tags it carries deletes it completely (optional: leave it in the backend for now, or tag it as "trash")
- the view (if at all easily possible) should keep the semantics of hard links to the root visible

Controller
----------



Corner Cases
------------
- what happens if in the backend directory there is a file which doesn't show up in the db? will it be invisible but impossible to create? will it be there but untagged? (maybe in an "untagged" dir?)

Questions
---------
- do dirs always have to be created manually(fuse limitation)? (I.e., does fuse do the same thing as plain "ls" to verify that dir is actually there? Or can we simply write to a non-existing dir and have our backend create it on the fly? (For now: manual creation)
