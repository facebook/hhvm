---
title: Lock
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A File Lock, which is unlocked as a disposable




To acquire one, call ` lock `
on a Base object.




Note that in some cases, such as the non-blocking lock types, we may throw
an ` LockAcquisitionException ` instead of acquiring the lock. If this
is not desired behavior it should be guarded against.




## Interface Synopsis




``` Hack
namespace HH\Lib\File;

final class Lock implements \IDisposable {...}
```




### Public Methods




+ [` ->__construct(\HH\Lib\OS\FileDescriptor $fd) `](/hsl/Classes/HH.Lib.File/Lock/__construct/)
+ [` ->__dispose(): void `](/hsl/Classes/HH.Lib.File/Lock/__dispose/)
<!-- HHAPIDOC -->
