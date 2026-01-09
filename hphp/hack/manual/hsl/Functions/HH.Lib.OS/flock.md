
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Acquire or remove an advisory lock on a file descriptor




``` Hack
namespace HH\Lib\OS;

function flock(
  FileDescriptor $fd,
  int $flags,
): void;
```




See ` man 2 flock ` for details. On error, an `` ErrnoException `` will be thrown.




A shared lock can also be 'upgraded' to an exclusive lock, however this
operation is not guaranteed to be atomic: systems may implement this by
releasing the shared lock, then attempting to acquire an exclusive lock. This
may lead to an upgrade attempt meaning that a lock is lost entirely, without
a replacement, as another process may potentially acquire a lock between
these operations.




## Parameters




+ ` FileDescriptor $fd `
+ ` int $flags ` a bitmask of `` LOCK_ `` flags; one out of ``` LOCK_EX ```, ```` LOCK_SH ````, or
  ````` LOCK_UN ````` **must** be specified.




## Returns




* ` void `
<!-- HHAPIDOC -->
