---
pageid: expr.name
title: name & iname
layout: docs
section: Expression Terms
permalink: docs/expr/name.html
redirect_from: docs/expr/name/
---

The `name` expression performs exact matches against file names.  By default it
is scoped to the basename of the file:

    ["name", "Makefile"]

You may specify multiple names to match against by setting the second argument
to an array:

    ["name", ["foo.txt", "Makefile"]]

This second form can be accelerated and is preferred over an `anyof`
construction.

You may change the scope of the match via the optional third argument:

    ["name", "path/to/file.txt", "wholename"]
    ["name", ["path/to/one", "path/to/two"], "wholename"]

Finally, you may specify case insensitive evaluation by using `iname` instead
of `name`.

*Since 2.9.9.*

Starting in version 2.9.9, on macOS systems where the watched root is a case
insensitive filesystem (this is the common case for macOS), `name` is equivalent
to `iname`.

*Since 4.7.*

You can override the case sensitivity of all name matching operations used
in the query by setting the `case_sensitive` field in your query.

