---
pageid: expr.dirname
title: dirname & idirname
layout: docs
section: Expression Terms
permalink: docs/expr/dirname.html
redirect_from: docs/expr/dirname/
---

*Since version 3.1*

The `dirname` term allows matching on the parent directory structure for a
given file.

For the examples below, given a file with a wholename (the relative path from
the project root) of `foo/bar/baz`, the dirname portion is `foo/bar`.

The following two terms will match any file whose dirname is either exactly a
match for `foo/bar` or is any child directory of `foo/bar`.  The first of these
two is a shortcut for the second:

      ["dirname", "foo/bar"]
      ["dirname", "foo/bar", ["depth", "ge", 0]]

The second of those terms uses a relational expression based on the depth of
the file within the specified dirname.  A file is considered to have
`depth == 0` if it is contained directly within the specified dirname.  It has
`depth == 1` if it is contained in a direct child directory of the specified
dirname, `depth == 2` if it is contained in a grand-child directory and so on.

The relational expression accepts the same relational operators as described in
the [size term](size.html).

If you wanted to match only files that were directly in the `foo/bar` dir:

      ["dirname", "foo/bar", ["depth", "eq", 0]]

If you wanted to match only files that were in a grand-child or deeper:

      ["dirname", "foo/bar", ["depth", "ge", 2]]

`idirname` is the case insensitive version of `dirname`.  If the watched root
is detected as a case insensitive fileystem, `dirname` is equivalent to `idirname`.


