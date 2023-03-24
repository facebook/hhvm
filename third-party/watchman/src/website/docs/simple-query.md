---
title: Simple Pattern Syntax
section: Queries
---

Simple patterns follow a more traditional UNIX command line approach of using
command line switches to indicate the nature of the pattern match. When simple
patterns are used, the result set unconditionally includes all core file
metadata fields. They are described in more detail below.

## Simple Pattern syntax

Where you see `[patterns]` in the command syntax for the `find`, `since` and
`trigger` commands, we allow filename patterns that match according the
following rules:

- We maintain an _inclusion_ and an _exclusion_ list. As the arguments are
  processed we'll accumulate them in one or the other. By default they are
  accumulated into the _inclusion_ list.
- `-X` causes any subsequent items to be placed into the _exclusion_ list
- `-I` causes any subsequent items to be placed into the _inclusion_ list
- `--` indicates the end of the set of patterns
- `-p` indicates that the following pattern should use `pcre` as the expression
  term. This is reset after generating the next term.
- `-P` indicates that the following pattern should use `ipcre` as the expression
  term and perform a case insensitive match. This is reset after generating the
  next term.
- If neither `-p` nor `-P` were used, the generated term will use `match`
- `!` followed by a space followed by a pattern will negate the sense of the
  pattern match generating a `not` term.

Any elements in the inclusion list will match; they are composed together using
an "anyof" term.

The inclusion list and exclusion lists are composed using the logic
`(NOT anyof exclusion) AND (anyof inclusion)`.

For example:

     '*.c'

Generates a file expression:

```json
["match", "*.c", "wholename"]
```

A list:

    '*.js' '*.css'

```json
["anyof",
  ["match", "*.js", "wholename"],
  ["match", "*.css", "wholename"]
]
```

An example of how the exclusion list syntax works:

     -X '*.c' -I '*main*'

Generates:

```json
["allof",
  ["not", ["match", "*.c", "wholename"]],
  ["match", "*main*", "wholename"]
]
```
