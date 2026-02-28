# Query Synchronization

**Terminology note**: Like the C++ memory model, this document will use the term sequenced-before and sequenced-after to reflect an ordering relationship between operations. Not all operations have a defined order. If two separate processes write to the filesystem, even if one does complete prior to the other, it does not necessarily imply a sequenced relationship between them. If one process writes two files on the same thread, there is a sequenced relationship.

## Overview

Watchman provides a model of query consistency: queries sequenced-after a filesystem modification will reflect those modifications in its query results.

That is, if a file is modified, a subsequent query should observe those modifications.

The challenge is that filesystem notification APIs are asynchronous and deferred. So how can Watchman know when it has processed every notification that has happened prior?

The specifics are platform-dependent, but Watchman generally uses cookie files. It generates a file of a unique filename in the watched directory and waits to receive the corresponding notification, at which point it assumes it has observed every prior change event.

## inotify

In practice, cookie files work well on Linux. It's unclear whether this sequencing is guaranteed by the kernel, however. That is, is it possible for a write to file A somewhere in the directory hierarchy followed by a write to a cookie file elsewhere to be reported in the opposite order?

inotify's own man page says:

> However, robust applications should allow for the fact that bugs in the monitoring logic or races of the kind described below may leave the cache inconsistent with the filesystem state. It is probably wise to do some consistency checking, and rebuild the cache when inconsistencies are detected.

In addition:

> The events returned by reading from an inotify file descriptor form an ordered queue.  Thus, for example, it is guaranteed that when renaming from one directory to another, events will be produced in the correct order on the inotify file descriptor.

Linux kernel 5.3 [introduced a regression in inotify](https://lore.kernel.org/linux-fsdevel/CAOQ4uxhWz_J4fir9ft5XpRVHoNCdk_bP1y-a=MhBqRYSf3N8gA@mail.gmail.com/) (likely 49246466a989 "fsnotify: move fsnotify_nameremove() hook out of d_delete()") that caused events to sometimes be reported prior to the corresponding caches being flushed after an unlink, resulting in Watchman sometimes observing stale data.

Under correct operation, inotify should carry sequencing through each step. That is, the inotify event's observation is sequenced-after its corresponding write. The regression here broke that.

## FSEvents

There are three mechanisms for watching for filesystem changes on macOS: kqueue, /dev/fsevents, and CoreServices FSEvents.

kqueue does not work for large repositories, as it requires a large number of file descriptors which macOS is not tolerant of.

> Last time I tried I had to go into recovery mode to stop the endless reboot cycle where macOS kernel panicked due to not having any FDs available... I tried raising the max FDs to something like 128 million, but 1) this did nothing, 2) some applications were looping through all the potential FDs to try to close them.

/dev/fsevents requires root and is a private, undocumented API.

Apple encourages use of FSEvents, which has two modes: per-file notifications, and per-directory notifications. The former produces a higher volume of change events, but in theory allows Watchman to synchronize with cookies as in inotify. The latter produces fewer change events, and in turn requires Watchman `stat()` every child of a directory when a change notification arrives.

Here is our understanding:

```
┌──────────┐    ┌───────────┐    ┌───────────────┐
│ Watchman │◄──►│ fseventsd │◄──►│ Darwin kernel │
└──────────┘    └───────────┘    └───────────────┘
```

The kernel reports change events to fseventsd which holds them for a period of time. They may be coalesced, and coalesced events are then reported to subscribed processes like Watchman. The coalesced events may be coarse. The FSEvents documentation is not clear, but we assume it's possible for macOS to combine multiple changes in one directory into a "just scan this directory" event.

FSEvents provides a mechanism for flushing change events: [FSEventStreamFlushSync](https://developer.apple.com/documentation/coreservices/1445629-fseventstreamflushsync?language=objc) and [FSEventStreamFlushAsync](https://developer.apple.com/documentation/coreservices/1441727-fseventstreamflushasync?language=objc). This only seems to ensure that events queued in fseventsd will be flushed to subscribers, but does not provide any guarantees about events pending in the kernel.

_As far as we can tell, there is no reliable way to synchronize queries on macOS._

Imagine the following sequence of events:

* Watchman starts up and crawls the directory.
* A program (e.g. git checkout) writes a large number of files into that directory.
* The program finishes, and then immediately issues a Watchman query, expecting all of those changes to be correct in the result set.
* That is, the query must not begin evaluating until every notification produced prior has been observed.

### Directory Notifications

Because Watchman does not receive notifications about individual files in this mode, it will only observe the cookie by scanning a changed directory. Therefore, Watchman cannot distinguish between discovery of a cookie for an unrelated reason (e.g. processing events from before the query started) and being notified of its creation.

One option would be to try using cookie _directories_ instead of files, and adding entries to these directories in order to produce events. This would work if

1. we were confident FSEvents provided a guarantee that changes sequenced-after each other are notified in the same order, and
2. we were confident these cookie directory events would never be coalesced into a scan.

We are not confident of either. See the next section.

### File Notifications

In production, our Watchman was instrumented with two ring buffers: one containing the last N events provided by FSEvents and the other the paths that Watchman has `stat()`'d. In addition, we record when Watchman noticed a cookie file and unblocked the corresponding query.

I have observed the following scenario in production:

1. src/foo.cpp changes from size 100 to size 101
2. Watchman query begins
3. Watchman writes cookie file into repository
4. FSEvents notifies Watchman of the cookie
5. Watchman notices the cookie, and starts FSEventStreamFlushSync
6. FSEventStreamFlushSync succeeds. Watchman assumes at this point it is caught up and unblocks the query, which returns size "100" for src/foo.cpp
6. **Then**, FSEvents notifies Watchman that src/foo.cpp has changed

### Do we have any options?

If fseventsd is inflating per-file notifications from a directory-only stream of events from the kernel, it's possible directory changes do have an ordering.

## Detecting Divergence

Watchman's view of the filesystem can diverge temporarily or persistently. The aforementioned Linux inotify regression was persistent. FSEvents on the other hand just causes queries to be unblocked too soon. Subsequent queries will eventually see the correct filesystem state.

Both circumstances can be discovered with `watchmanctl audit`, which rapidly scans the filesystem in parallel with a Watchman query and then compares the results.
