---
title: pcre & ipcre
category: Expression Terms
---

_To use this feature, you must configure watchman `--with-pcre`!_

The `pcre` expression performs a Perl Compatible Regular Expression match
against the basename of the file. This pattern matches `test_plan.php` but not
`mytest_plan`:

    ["pcre", "^test_"]

You may optionally provide a third argument to change the scope of the match
from the basename to the wholename of the file.

    ["pcre", "txt", "basename"]
    ["pcre", "txt", "wholename"]

`pcre` is case sensitive; for case insensitive matching use `ipcre` instead; it
behaves identically to `pcre` except that the match is performed ignoring case.

_Since 2.9.9._

Starting in version 2.9.9, on macOS systems where the watched root is a case
insensitive filesystem (this is the common case for macOS), `pcre` is equivalent
to `ipcre`.

_Since 4.7._

You can override the case sensitivity of all name matching operations used in
the query by setting the `case_sensitive` field in your query.
