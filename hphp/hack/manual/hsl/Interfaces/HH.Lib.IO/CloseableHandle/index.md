---
title: CloseableHandle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A non-disposable handle that is explicitly closeable




Some handles, such as those returned by ` IO\server_error() ` may
be neither disposable nor closeable.




## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

interface CloseableHandle implements Handle {...}
```




### Public Methods




+ [` ->close(): void `](</hsl/Interfaces/HH.Lib.IO/CloseableHandle/close/>)\
  Close the handle
+ [` ->closeWhenDisposed(): \IDisposable `](</hsl/Interfaces/HH.Lib.IO/CloseableHandle/closeWhenDisposed/>)\
  Close the handle when the returned disposable is disposed
<!-- HHAPIDOC -->
