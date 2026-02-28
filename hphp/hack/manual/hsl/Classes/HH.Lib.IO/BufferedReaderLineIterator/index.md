---
title: BufferedReaderLineIterator
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

final class BufferedReaderLineIterator implements AsyncIterator<string> {...}
```




### Public Methods




+ [` ->__construct(BufferedReader $reader) `](/hsl/Classes/HH.Lib.IO/BufferedReaderLineIterator/__construct/)
+ [` ->next(): Awaitable<?(mixed, string)> `](/hsl/Classes/HH.Lib.IO/BufferedReaderLineIterator/next/)
<!-- HHAPIDOC -->
