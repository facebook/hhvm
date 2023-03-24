---
pageid: expr.suffix
title: suffix
layout: docs
section: Expression Terms
permalink: docs/expr/suffix.html
redirect_from: docs/expr/suffix/
---

The `suffix` expression evaluates true if the file suffix matches the second
argument.  This matches files name `foo.php` and `foo.PHP` but not `foophp`:

    ["suffix", "php"]

Suffix expression matches are case insensitive.

## suffix-set

*Since 5.0*

You may specify multiple suffixes to match against by setting the second argument
to an array:

    ["suffix", ["php", "css", "html"]]

This second form can be accelerated and is preferred over an `anyof`
construction.  In the following example the two terms are functionally
equivalent but the set form has a more efficient and thus faster runtime:

    ["anyof", ["suffix", "php"], ["suffix", "html"]]

    ["suffix", ["php", "html"]]

The [capability](/watchman/docs/capabilities.html) name associated with this
enhanced functionality is `suffix-set`.
