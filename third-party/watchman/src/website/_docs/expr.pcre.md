---
pageid: expr.pcre
title: pcre & ipcre
layout: docs
section: Expression Terms
permalink: docs/expr/pcre.html
redirect_from: docs/expr/pcre/
---

*To use this feature, you must configure watchman `--with-pcre`!*

The `pcre` expression performs a Perl Compatible Regular Expression match
against the basename of the file.  This pattern matches `test_plan.php` but not
`mytest_plan`:

    ["pcre", "^test_"]

You may optionally provide a third argument to change the scope of the match
from the basename to the wholename of the file.

    ["pcre", "txt", "basename"]
    ["pcre", "txt", "wholename"]

`pcre` is case sensitive; for case insensitive matching use `ipcre` instead;
it behaves identically to `pcre` except that the match is performed ignoring
case.

*Since 2.9.9.*

Starting in version 2.9.9, on macOS systems where the watched root is a case
insensitive filesystem (this is the common case for macOS), `pcre` is equivalent
to `ipcre`.

*Since 4.7.*

You can override the case sensitivity of all name matching operations used
in the query by setting the `case_sensitive` field in your query.

