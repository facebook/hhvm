---
title: ConnectionException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
namespace HH\Lib\OS;

abstract class ConnectionException extends ErrnoException {...}
```




### Public Methods ([` HH\Lib\OS\ErrnoException `](/hsl/Classes/HH.Lib.OS/ErrnoException/))




+ [` ->__construct(Errno $errno, string $message) `](/hsl/Classes/HH.Lib.OS/ErrnoException/__construct/)
+ [` ->getCode(): Errno `](/hsl/Classes/HH.Lib.OS/ErrnoException/getCode/)\
  Deprecated for clarity, and potential future ambiguity
+ [` ->getErrno(): Errno `](/hsl/Classes/HH.Lib.OS/ErrnoException/getErrno/)
<!-- HHAPIDOC -->
