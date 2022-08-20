---
pageid: expr.since
title: since
layout: docs
section: Expression Terms
permalink: docs/expr/since.html
redirect_from: docs/expr/since/
---

Evaluates as true if the specified time property of the file is greater than
the since value.  Note that this is not the same as the `since` generator; when
used as an expression term we are performing a straight clockspec comparison.
When used as a generator, candidate files are selected based on the `since`
time index.  The end result might or might not be the same--in particular, if
the `since` time index is not passed in, it will be treated the same as a fresh
instance, and only files that exist will be returned. The efficiency can vary
based on the size and shape of the file tree that you are watching; it may be
cheaper to generate the candidate set of files by suffix and then check the
modification time if many files were changed since your last query.

This will yield a true value if the observed change time is more recent than
the specified clockspec (this is equivalent to specifying "oclock" as the third
parameter):

     ["since", "c:12345:234"]

You may specify particular fields from the filesystem metadata.  In this case
your clockspec should be a unix time value:

     ["since", 12345668, "mtime"]
     ["since", 12345668, "ctime"]

You may explicitly request the observed clock values too; in these cases we'll
accept either a timestamp or a clock value.  The `oclock` is the last observed
change clock value (observed clock) and the `cclock` is the clock value where
we first observed the file come into existence (created clock):

     ["since", 12345668, "oclock"]
     ["since", "c:1234:123", "oclock"]
     ["since", 12345668, "cclock"]
     ["since", "c:1234:2342", "cclock"]

