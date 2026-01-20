---
title: CloseableFDHandle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

interface CloseableFDHandle implements FDHandle, CloseableHandle {...}
```




### Public Methods ([` HH\Lib\IO\FDHandle `](</hsl/Interfaces/HH.Lib.IO/FDHandle/>))




+ [` ->getFileDescriptor(): \HH\Lib\OS\FileDescriptor `](</hsl/Interfaces/HH.Lib.IO/FDHandle/getFileDescriptor/>)







### Public Methods ([` HH\Lib\IO\CloseableHandle `](</hsl/Interfaces/HH.Lib.IO/CloseableHandle/>))




* [` ->close(): void `](</hsl/Interfaces/HH.Lib.IO/CloseableHandle/close/>)\
  Close the handle
* [` ->closeWhenDisposed(): \IDisposable `](</hsl/Interfaces/HH.Lib.IO/CloseableHandle/closeWhenDisposed/>)\
  Close the handle when the returned disposable is disposed
<!-- HHAPIDOC -->
