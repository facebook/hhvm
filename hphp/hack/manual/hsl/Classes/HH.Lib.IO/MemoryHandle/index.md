---
title: MemoryHandle
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Read from/write to an in-memory buffer




This class is intended for use in unit tests.




## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

final class MemoryHandle implements CloseableSeekableReadWriteHandle {...}
```




### Public Methods




+ [` ->__construct(string $buffer = '', MemoryHandleWriteMode $writeMode = MemoryHandleWriteMode::OVERWRITE) `](/hsl/Classes/HH.Lib.IO/MemoryHandle/__construct/)
+ [` ->appendToBuffer(string $data): void `](/hsl/Classes/HH.Lib.IO/MemoryHandle/appendToBuffer/)\
  Append data to the internal buffer, preserving position
+ [` ->close(): void `](/hsl/Classes/HH.Lib.IO/MemoryHandle/close/)
+ [` ->closeWhenDisposed(): \IDisposable `](/hsl/Classes/HH.Lib.IO/MemoryHandle/closeWhenDisposed/)
+ [` ->getBuffer(): string `](/hsl/Classes/HH.Lib.IO/MemoryHandle/getBuffer/)
+ [` ->readAllowPartialSuccessAsync(?int $max_bytes = NULL, ?int $_timeout_nanos = NULL): Awaitable<string> `](/hsl/Classes/HH.Lib.IO/MemoryHandle/readAllowPartialSuccessAsync/)
+ [` ->readImpl(?int $max_bytes = NULL): string `](/hsl/Classes/HH.Lib.IO/MemoryHandle/readImpl/)
+ [` ->reset(string $data = ''): void `](/hsl/Classes/HH.Lib.IO/MemoryHandle/reset/)\
  Set the internal buffer and reset position to the beginning of the file
+ [` ->seek(int $pos): void `](/hsl/Classes/HH.Lib.IO/MemoryHandle/seek/)
+ [` ->tell(): int `](/hsl/Classes/HH.Lib.IO/MemoryHandle/tell/)
+ [` ->writeAllowPartialSuccessAsync(string $data, ?int $timeout_nanos = NULL): Awaitable<int> `](/hsl/Classes/HH.Lib.IO/MemoryHandle/writeAllowPartialSuccessAsync/)







### Public Methods ([` HH\Lib\IO\WriteHandle `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/>))




* [` ->writeAllAsync(string $bytes, ?int $timeout_ns = NULL): Awaitable<void> `](</hsl/Interfaces/HH.Lib.IO/WriteHandle/writeAllAsync/>)\
  Write all of the requested data







### Public Methods ([` HH\Lib\IO\ReadHandle `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/>))




- [` ->readAllAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllAsync/>)\
  Read until there is no more data to read
- [` ->readFixedSizeAsync(int $size, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readFixedSizeAsync/>)\
  Read a fixed amount of data







### Protected Methods




+ [` ->writeImpl(string $data): int `](/hsl/Classes/HH.Lib.IO/MemoryHandle/writeImpl/)
<!-- HHAPIDOC -->
