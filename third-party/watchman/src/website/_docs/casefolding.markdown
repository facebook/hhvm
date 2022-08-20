---
pageid: casefolding
title: Case-Insensitivity
layout: docs
section: Internals
permalink: docs/casefolding.html
redirect_from: docs/casefolding/
---

Watchman is currently completely unaware of case-insensitivity in file systems,
and does not attempt to do any case-folding of file names. On a case-insensitive
file system like Mac macOS's [HFS+](https://en.wikipedia.org/wiki/HFS_Plus), this
can manifest itself in different ways:

* If a file `foo.txt` is renamed to `FOO.txt`, Watchman will report `FOO.txt` as
  created and `foo.txt` separately as changed.
* If a file `foo.txt` is removed and another file `FOO.txt` is later added,
  Watchman will report `FOO.txt` as added, but it might report `foo.txt` as
  either removed or changed.

In general, both `foo.txt` and `FOO.txt` can be reported, sometimes with
different stat data, sometimes with the same stat data.

## Why doesn't Watchman support case-folding properly?

One problem is that 'properly' is hard to pin down. There are at least four
levels of correctness here:

- handle ASCII case-folding only (95% solution)
- handle ASCII + accented ASCII case-folding only (98%)
- full handling of current Unicode spec using a Unicode database (99%)
- using the special folding table written to a hidden file on disk at file
system creation time that matches Apple's interpretation of Unicode at the time
of the OS release + their own quirks (100%)

Clients of Watchman might have their own idea of case-folding, which might or
might not be compatible with Watchman's idea of it. So far, clients have managed
to handle case-folding outside of Watchman, with some success.

## Does this matter?

It depends on your application.

**Example 1:** Your application is a build system that has a pre-baked list of
files. Your application expects files to be on disk in the correct case even on
case-insensitive file systems, and you declare that the behavior is undefined if
they aren't. You invoke Watchman by asking it what files have changed. In this
case, Watchman should work without you having to do anything special.

**Example 2:** Your application is a build system rule to generate CSS rules
that is run by a Watchman trigger on `*.scss`. You expect all files you care
about to end with the string `.scss` on case-insensitive file systems, and not
another variant of it like `.SCSS`. In this case, Watchman should work fine --
at most, it will provide you the same file multiple times with different case
variants. You might be dealing with that in your build system anyway.

**Example 3:** Like example 2, except you expect `.SCSS` and other variants to
work too. In that case the only way is to explicitly add all possible variants
to the trigger rule.

**Example 4:** You're a source control system that has its own ideas about
case-folding that might or might not match up with the operating system's. You
perform case-folding against an internal data structure, so that if the data
structure has `foo.txt` and the file system has `FOO.txt` you make `foo.txt`
take precedence. In that case, Watchman will tell you about both `FOO.txt` and
`foo.txt`, and it's up to you to perform
normalization. [hgwatchman](https://bitbucket.org/facebook/hgwatchman) just
consults the file system in the rare case that a file changes case.

## Credits

The levels of correctness were proposed by Matt Mackall <mpm@selenic.com>.
