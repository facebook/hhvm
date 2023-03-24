---
title: since
section: Commands
---

~~~bash
$ watchman since /path/to/dir <clockspec> [patterns]
~~~

Finds all files that were modified since the specified clockspec that
match the optional list of patterns.  If no patterns are specified,
all modified files are returned.

The response includes a `files` array, each element of which is an
object with fields containing information about the file:

~~~json
{
    "version": "2.7",
    "is_fresh_instance": true,
    "clock": "c:80616:59",
    "files": [
        {
            "cclock": "c:80616:1",
            "ctime": 1357617635,
            "dev": 16777220,
            "exists": true,
            "gid": 100,
            "ino": 20161390,
            "mode": 33188,
            "mtime": 1357617635,
            "name": "argv.c",
            "nlink": 1,
            "oclock": "c:80616:39",
            "size": 1340,
            "uid": 100
        }
    ]
}
~~~

The fields should be largely self-explanatory; they correspond to
fields from the underlying `struct stat`, but a couple need special
mention:

 * **cclock** - The "created" clock; the clock value representing the time that
this file was first observed, or the clock value where this file changed from
deleted to non-deleted state.
 * **oclock** - The "observed" clock; the clock value representing the time
that this file was last observed to have changed.
 * **exists** - whether we believe that the file exists on disk or not.  If
this is false, most of the other fields will be omitted.
 * **new** - this is only set in cases where the file results were generated as
part of a time or clock based query, such as the `since` command.  If the
`cclock` value for the file is newer than the time you specified then the file
entry is marked as `new`.  This allows you to more easily determine if the file
was newly created without having to maintain a lot of state.
