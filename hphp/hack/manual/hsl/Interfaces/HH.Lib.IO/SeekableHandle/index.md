---
title: SeekableHandle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A handle that can have its' position changed




## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

interface SeekableHandle implements Handle {...}
```




### Public Methods




+ [` ->seek(int $offset): void `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/seek/>)\
  Move to a specific offset within a handle
+ [` ->tell(): int `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/tell/>)\
  Get the current pointer position within a handle
<!-- HHAPIDOC -->
