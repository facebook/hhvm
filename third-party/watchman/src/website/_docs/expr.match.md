---
pageid: expr.match
title: match & imatch
layout: docs
section: Expression Terms
permalink: docs/expr/match.html
redirect_from: docs/expr/match/
---

The `match` expression performs a glob-style match against the basename of
the file, evaluating true if the match is successful.

```json
["match", "*.txt"]
```

You may optionally provide a third argument to change the scope of the match
from the basename to the wholename of the file.

```json
["match", "*.txt", "basename"]
["match", "dir/*.txt", "wholename"]
```

### Case sensitivity

`match` is case sensitive; for case insensitive matching use `imatch` instead;
it behaves identically to `match` except that the match is performed ignoring
case.

*Since 2.9.9.*

On systems where the watched root is a case insensitive filesystem (this is the
common case for macOS and Windows), `match` is equivalent to `imatch`.

*Since 4.7.*

You can override the case sensitivity of all name matching operations used
in the query by setting the `case_sensitive` field in your query.

## wildmatch

*Since 3.7.*

The `match` expression has been enhanced as described below.  The
[capability](/watchman/docs/capabilities.html) name associated with this
enhanced functionality is `wildmatch`.

If you want to recursively match all files under a directory, use the `**`
glob operator along with the `wholename` scope:

```json
["match", "src/**/*.java", "wholename"]
```

By default, paths whose names start with `.` are not included. To
change this behavior, you may optionally provide a fourth argument
containing a dictionary of flags:

```json
["match", "*.txt", "basename", {"includedotfiles": true}]
```

By default, backslashes in the pattern escape the next character, so
`\*` matches a literal `*` character. To change this behavior so
backslashes are treated literally, set the `noescape` flag to `true`
in the flags dictionary. (Note that `\\` is a literal `\` in JSON notation):

```json
["match", "*\\*.txt", "filename", {"noescape": true}]
```

matches `a\b.txt`.

