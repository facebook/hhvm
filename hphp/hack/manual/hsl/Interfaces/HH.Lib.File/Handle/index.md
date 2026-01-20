---
title: Handle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
namespace HH\Lib\File;

interface Handle implements \HH\Lib\IO\SeekableFDHandle {...}
```




### Public Methods




+ [` ->getPath(): string `](</hsl/Interfaces/HH.Lib.File/Handle/getPath/>)\
  Get the name of this file
+ [` ->getSize(): int `](</hsl/Interfaces/HH.Lib.File/Handle/getSize/>)\
  Get the size of the file
+ [` ->lock(LockType $type): Lock `](</hsl/Interfaces/HH.Lib.File/Handle/lock/>)\
  Get a shared or exclusive lock on the file
+ [` ->tryLockx(LockType $type): Lock `](</hsl/Interfaces/HH.Lib.File/Handle/tryLockx/>)\
  Immediately get a shared or exclusive lock on a file, or throw







### Public Methods ([` HH\Lib\IO\FDHandle `](</hsl/Interfaces/HH.Lib.IO/FDHandle/>))




* [` ->getFileDescriptor(): \HH\Lib\OS\FileDescriptor `](</hsl/Interfaces/HH.Lib.IO/FDHandle/getFileDescriptor/>)







### Public Methods ([` HH\Lib\IO\SeekableHandle `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/>))




- [` ->seek(int $offset): void `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/seek/>)\
  Move to a specific offset within a handle
- [` ->tell(): int `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/tell/>)\
  Get the current pointer position within a handle
<!-- HHAPIDOC -->
