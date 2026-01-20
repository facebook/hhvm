---
title: ReadWriteHandle
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

interface ReadWriteHandle implements WriteHandle, ReadHandle, \HH\Lib\IO\SeekableReadWriteFDHandle {...}
```




### Public Methods ([` HH\Lib\File\WriteHandle `](</hsl/Interfaces/HH.Lib.File/WriteHandle/>))




+ [` ->truncate(?int $length = NULL): void `](</hsl/Interfaces/HH.Lib.File/WriteHandle/truncate/>)







### Public Methods ([` HH\Lib\File\Handle `](</hsl/Interfaces/HH.Lib.File/Handle/>))




* [` ->getPath(): string `](</hsl/Interfaces/HH.Lib.File/Handle/getPath/>)\
  Get the name of this file
* [` ->getSize(): int `](</hsl/Interfaces/HH.Lib.File/Handle/getSize/>)\
  Get the size of the file
* [` ->lock(LockType $type): Lock `](</hsl/Interfaces/HH.Lib.File/Handle/lock/>)\
  Get a shared or exclusive lock on the file
* [` ->tryLockx(LockType $type): Lock `](</hsl/Interfaces/HH.Lib.File/Handle/tryLockx/>)\
  Immediately get a shared or exclusive lock on a file, or throw







### Public Methods ([` HH\Lib\IO\FDHandle `](</hsl/Interfaces/HH.Lib.IO/FDHandle/>))




- [` ->getFileDescriptor(): \HH\Lib\OS\FileDescriptor `](</hsl/Interfaces/HH.Lib.IO/FDHandle/getFileDescriptor/>)







### Public Methods ([` HH\Lib\IO\SeekableHandle `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/>))




+ [` ->seek(int $offset): void `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/seek/>)\
  Move to a specific offset within a handle
+ [` ->tell(): int `](</hsl/Interfaces/HH.Lib.IO/SeekableHandle/tell/>)\
  Get the current pointer position within a handle







### Public Methods ([` HH\Lib\IO\WriteHandle `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/>))




* [` ->writeAllAsync(string $bytes, ?int $timeout_ns = NULL): Awaitable<void> `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/writeAllAsync/>)\
  Write all of the requested data
* [` ->writeAllowPartialSuccessAsync(string $bytes, ?int $timeout_ns = NULL): Awaitable<int> `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/writeAllowPartialSuccessAsync/>)\
  Write data, waiting if necessary







### Public Methods ([` HH\Lib\IO\ReadHandle `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/>))




- [` ->readAllAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllAsync/>)\
  Read until there is no more data to read
- [` ->readAllowPartialSuccessAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllowPartialSuccessAsync/>)\
  Read from the handle, waiting for data if necessary
- [` ->readFixedSizeAsync(int $size, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readFixedSizeAsync/>)\
  Read a fixed amount of data
- [` ->readImpl(?int $max_bytes = NULL): string `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readImpl/>)\
  An immediate, unordered read







### Protected Methods ([` HH\Lib\IO\WriteHandle `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/>))




+ [` ->writeImpl(string $bytes): int `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/writeImpl/>)\
  An immediate unordered write
<!-- HHAPIDOC -->
