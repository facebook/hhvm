---
title: HErrnoException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Class for exceptions reported via the C ` h_errno ` variable




## Interface Synopsis




``` Hack
namespace HH\Lib\OS;

final class HErrnoException extends \Exception {...}
```




### Public Methods




+ [` ->__construct(HErrno $errno, string $message) `](/hsl/Classes/HH.Lib.OS/HErrnoException/__construct/)
+ [` ->getHErrno(): HErrno `](/hsl/Classes/HH.Lib.OS/HErrnoException/getHErrno/)
<!-- HHAPIDOC -->
