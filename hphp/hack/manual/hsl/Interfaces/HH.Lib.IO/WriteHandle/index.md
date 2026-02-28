---
title: WriteHandle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An interface for a writable Handle




Order of operations is guaranteed, *except* for ` writeImplBlocking `;
`` writeImplBlocking() `` will immediately try to write to the handle.




## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

interface WriteHandle implements Handle {...}
```




### Public Methods




+ [` ->writeAllAsync(string $bytes, ?int $timeout_ns = NULL): Awaitable<void> `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/writeAllAsync/>)\
  Write all of the requested data
+ [` ->writeAllowPartialSuccessAsync(string $bytes, ?int $timeout_ns = NULL): Awaitable<int> `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/writeAllowPartialSuccessAsync/>)\
  Write data, waiting if necessary







### Protected Methods




* [` ->writeImpl(string $bytes): int `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/writeImpl/>)\
  An immediate unordered write
<!-- HHAPIDOC -->
