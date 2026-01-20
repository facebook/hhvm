---
title: ReadHandle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An [` IO\Handle `](</hsl/Interfaces/HH.Lib.IO/Handle/>) that is readable




If implementing this interface, you may wish to use
` ReadHandleConvenienceAccessorTrait `, which implements [` readAllAsync() `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllAsync/>) and
[` readFixedSizeAsync() `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readFixedSizeAsync/>) on top of `` readAsync ``.




## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

interface ReadHandle implements Handle {...}
```




### Public Methods




+ [` ->readAllAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllAsync/>)\
  Read until there is no more data to read
+ [` ->readAllowPartialSuccessAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllowPartialSuccessAsync/>)\
  Read from the handle, waiting for data if necessary
+ [` ->readFixedSizeAsync(int $size, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readFixedSizeAsync/>)\
  Read a fixed amount of data
+ [` ->readImpl(?int $max_bytes = NULL): string `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readImpl/>)\
  An immediate, unordered read
<!-- HHAPIDOC -->
