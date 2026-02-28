---
title: AlreadyLockedException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Indicates that a lock failed, because the file is already locked




This class does not extend [` OS\ErrnoException `](/hsl/Classes/HH.Lib.OS/ErrnoException/) as an `` EWOULDBLOCK `` after
``` flock($fd, LOCK_NB) ``` is expected rather than an error; this exception is
thrown when the caller has explicitly requested an exception for these cases.




## Interface Synopsis




``` Hack
namespace HH\Lib\File;

final class AlreadyLockedException extends \Exception {...}
```



<!-- HHAPIDOC -->
