---
title: Socket
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A Unix socket for a server or client connection




## Interface Synopsis




``` Hack
namespace HH\Lib\Unix;

interface Socket implements \HH\Lib\Network\Socket {...}
```




### Public Methods ([` HH\Lib\Network\Socket `](</docs/hsl/Interfaces/HH.Lib.Network/Socket/>))




+ [` ->getLocalAddress(): this::TAddress `](</docs/hsl/Interfaces/HH.Lib.Network/Socket/getLocalAddress/>)\
  Returns the address of the local side of the socket
+ [` ->getPeerAddress(): this::TAddress `](</docs/hsl/Interfaces/HH.Lib.Network/Socket/getPeerAddress/>)\
  Returns the address of the remote side of the socket







### Public Methods ([` HH\Lib\IO\WriteHandle `](</docs/hsl/Interfaces/HH.Lib.IO/WriteHandle/>))




* [` ->writeAllAsync(string $bytes, ?int $timeout_ns = NULL): Awaitable<void> `](</docs/hsl/Interfaces/HH.Lib.IO/WriteHandle/writeAllAsync/>)\
  Write all of the requested data
* [` ->writeAllowPartialSuccessAsync(string $bytes, ?int $timeout_ns = NULL): Awaitable<int> `](</docs/hsl/Interfaces/HH.Lib.IO/WriteHandle/writeAllowPartialSuccessAsync/>)\
  Write data, waiting if necessary







### Public Methods ([` HH\Lib\IO\ReadHandle `](</docs/hsl/Interfaces/HH.Lib.IO/ReadHandle/>))




- [` ->readAllAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</docs/hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllAsync/>)\
  Read until there is no more data to read
- [` ->readAllowPartialSuccessAsync(?int $max_bytes = NULL, ?int $timeout_ns = NULL): Awaitable<string> `](</docs/hsl/Interfaces/HH.Lib.IO/ReadHandle/readAllowPartialSuccessAsync/>)\
  Read from the handle, waiting for data if necessary
- [` ->readFixedSizeAsync(int $size, ?int $timeout_ns = NULL): Awaitable<string> `](</docs/hsl/Interfaces/HH.Lib.IO/ReadHandle/readFixedSizeAsync/>)\
  Read a fixed amount of data
- [` ->readImpl(?int $max_bytes = NULL): string `](</docs/hsl/Interfaces/HH.Lib.IO/ReadHandle/readImpl/>)\
  An immediate, unordered read







### Protected Methods ([` HH\Lib\IO\WriteHandle `](</docs/hsl/Interfaces/HH.Lib.IO/WriteHandle/>))




+ [` ->writeImpl(string $bytes): int `](</docs/hsl/Interfaces/HH.Lib.IO/WriteHandle/writeImpl/>)\
  An immediate unordered write
<!-- HHAPIDOC -->
