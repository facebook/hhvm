---
title: Server
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
namespace HH\Lib\Unix;

final class Server implements \HH\Lib\Network\Server<CloseableSocket> {...}
```




### Public Methods




+ [` ::createAsync(string $path): Awaitable<this> `](/hsl/Classes/HH.Lib.Unix/Server/createAsync/)\
  Create a bound and listening instance
+ [` ->getLocalAddress(): string `](/hsl/Classes/HH.Lib.Unix/Server/getLocalAddress/)
+ [` ->nextConnectionAsync(): Awaitable<CloseableSocket> `](/hsl/Classes/HH.Lib.Unix/Server/nextConnectionAsync/)
+ [` ->stopListening(): void `](/hsl/Classes/HH.Lib.Unix/Server/stopListening/)
<!-- HHAPIDOC -->
