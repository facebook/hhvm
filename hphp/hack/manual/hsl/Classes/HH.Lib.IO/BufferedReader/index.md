---
title: BufferedReader
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Wrapper for ` ReadHandle `s, with buffered line-based byte-based accessors




+ [` readLineAsync() `](/hsl/Classes/HH.Lib.IO/BufferedReader/readLineAsync/) is similar to `` fgets() ``
+ [` readUntilAsync() `](/hsl/Classes/HH.Lib.IO/BufferedReader/readUntilAsync/) is a more general form
+ [` readByteAsync() `](/hsl/Classes/HH.Lib.IO/BufferedReader/readByteAsync/) is similar to `` fgetc() ``




## Interface Synopsis




``` Hack
namespace HH\Lib\IO;

final class BufferedReader implements ReadHandle {...}
```




### Public Methods




* [` ->__construct(ReadHandle $handle) `](/hsl/Classes/HH.Lib.IO/BufferedReader/__construct/)
* [` ->getHandle(): ReadHandle `](/hsl/Classes/HH.Lib.IO/BufferedReader/getHandle/)
* [` ->isEndOfFile(): bool `](/hsl/Classes/HH.Lib.IO/BufferedReader/isEndOfFile/)\
  If we are known to have reached the end of the file
* [` ->linesIterator(): AsyncIterator<string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/linesIterator/)\
  Iterate over all lines in the file
* [` ->readAllowPartialSuccessAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/readAllowPartialSuccessAsync/)
* [` ->readByteAsync(?int $timeout_ns = NULL): Awaitable<string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/readByteAsync/)\
  Read a single byte from the handle
* [` ->readFixedSizeAsync(int $size, ?int $timeout_ns = NULL): Awaitable<string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/readFixedSizeAsync/)
* [` ->readImpl(?int $max_bytes = NULL): string `](/hsl/Classes/HH.Lib.IO/BufferedReader/readImpl/)
* [` ->readLineAsync(): Awaitable<?string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/readLineAsync/)\
  Read until the platform end-of-line sequence is seen, or EOF is reached
* [` ->readLinexAsync(): Awaitable<string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/readLinexAsync/)\
  Read a line or throw EPIPE
* [` ->readUntilAsync(string $suffix): Awaitable<?string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/readUntilAsync/)\
  Read until the specified suffix is seen
* [` ->readUntilxAsync(string $suffix): Awaitable<string> `](/hsl/Classes/HH.Lib.IO/BufferedReader/readUntilxAsync/)\
  Read until the suffix, or raise EPIPE if the separator is not seen







### Public Methods ([` HH\Lib\IO\ReadHandle `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/>))




- [` ->readAllAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllAsync/>)\
  Read until there is no more data to read
<!-- HHAPIDOC -->
