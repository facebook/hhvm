---
title: Watchman | A file watching service
layout: home
permalink: index.html
---

Watchman exists to watch files and record when they change.  It can
also trigger actions (such as rebuilding assets) when matching files change.

### Concepts

 * Watchman can recursively watch one or more directory trees
   (we call them *roots*).
 * Watchman does not follow symlinks.  It knows they exist, but they show up
   the same as any other file in its reporting.
 * Watchman waits for a *root* to settle down before it will start
   to trigger notifications or command execution.
 * Watchman is conservative, preferring to err on the side of caution;
   it considers files to be freshly changed when you start to
   watch them or when it is unsure.
 * You can query a root for file changes since you last checked, or the
   current state of the tree
 * You can subscribe to file changes that occur in a root

### Quickstart

These two lines establish a watch on a source directory and then set up a
trigger named `buildme` that will run a tool named `minify-css` whenever a CSS
file is changed.  The tool will be passed a list of the changed filenames.

~~~bash
$ watchman watch ~/src
# the single quotes around '*.css' are important!
$ watchman -- trigger ~/src buildme '*.css' -- minify-css
~~~

The output for buildme will land in the Watchman log file unless you send it
somewhere else.
