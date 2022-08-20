---
pageid: cookies
title: Query Synchronization
layout: docs
category: Internals
permalink: docs/cookies.html
redirect_from: docs/cookies/
---

A file system monitor needs to make sure that queries see up-to-date
views. Watchman ensures that by creating a unique *cookie* for each query made
to it.

## Background

Consider a directory tree traversal to gather file status, such as the one
performed by `hg status` or `git status`. The traversal will race with any
operations happening concurrently, and this is impossible to fix. However, we
do get some weaker guarantees:

1. Every file operation that happens before the traversal is started will be
observed.
2. File operations that happen after the traversal is started may or may not be
observed.

For watchman, cookies enable us to provide similar guarantees. For a given
watchman query:

1. Every file operation that happens before the query is started will be
observed.
2. File operations that happen after the query is started may or may not be
observed.

## How cookies work

A *cookie* is a temporary file that is created inside a directory observed by
watchman. The cookie is created in a directory that is expected not to go
away. The obvious location is the root itself, but we'd like cookies not to show
up in VCS operations. So if a VCS directory (`.git`, `.hg` or `.svn`) is found,
that's where cookies are created instead.

The cookie is created while the root is locked, so watchman won't find the
cookie by accident while processing events from a prior run.

Once the cookie is created, the calling thread waits on a condition variable
guarded by the root's lock. This causes the lock to be released, and the root's
notify thread can now read events as usual.

When the notify thread finds that it is processing a cookie, it will signal its
respective condition variable. Importantly, this does not wake the calling
thread up immediately: since the notify thread still holds the root lock, the
calling thread will only be able to proceed once the notify thread releases the
lock.

*What do cookies get us?*

File monitoring systems like `inotify` typically provide an ordering guarantee:
notifications arrive in the order they happen. Any events happening before the
cookie is created will appear before the event for the cookie does, which means
they will be processed by the time the query is answered.

*How well do cookies work?*

The Mercurial test suite has proved to be a good stress test for
watchman. Before cookies were implemented, if 16 or more tests from the suite
were run in parallel, watchman would start falling behind and often produce
outdated answers. Cookies have successfully eradicated that.

*Can watchman find a cookie even if not all events leading to its appearance
 have been processed?*

Consider this situation when cookies are created inside `.hg`:

1. Event A happens that would cause `.hg` to be read recursively
2. Event B happens that touches a file `subdir/foo`
3. A cookie is created inside `.hg`, causing event C
4. Event A is read from the OS file notification system but not events B and C
5. The cookie is found but `subdir/foo` is never read.

On Linux, to prevent this from happening, watchman will only consider a cookie
to be found if it is directly returned via OS notifications. The only exception
to this is during the initial crawl or a recrawl, when the cookie directory
isn't being watched yet.

On other platforms, this becomes more complicated because the respective
monitoring system only tells us that something inside a directory was created,
not what was created. This is currently an unresolved issue.

## Limitation: macOS FSEvents

On macOS, Watchman uses
[FSEvents](https://developer.apple.com/documentation/coreservices/file_system_events)
to monitor filesystem changes. Watchman uses a combination of cookie files
described above and
[FSEventStreamFlushSync](https://developer.apple.com/documentation/coreservices/1445629-fseventstreamflushsync)
to attempt to catch up with all prior changes.

Unfortunately, in high-load situations like a large `git checkout` on a busy
host, we have observed that FSEvents from the `git checkout` may be received
after the cookie file notification and FSEventStreamFlushSync returning.

It turns out that FSEvents provides no guarantees here, and relying on it for
query synchronization is unsupported on any current Apple platform.  Their
suggested workaround is to implement a watcher with [Endpoint
Security](https://developer.apple.com/documentation/endpointsecurity).  Nobody
has evaluated the feasibility of this yet.

In the meantime, you can set a `settle_period` and `settle_timeout` on the
query. Both are integer milliseconds, and `settle_period` specifies the required
quiescence before the watcher is considered caught up.

## Credits

The idea was originally proposed by Matt Mackall <mpm@selenic.com>.
