---
title: Server
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Generic interface for a class able to accept socket connections




## Interface Synopsis




``` Hack
namespace HH\Lib\Network;

interface Server {...}
```




### Public Methods




+ [` ->getLocalAddress(): this::TAddress `](</hsl/Interfaces/HH.Lib.Network/Server/getLocalAddress/>)\
  Return the local (listening) address for the server
+ [` ->nextConnectionAsync(): Awaitable<TSock> `](</hsl/Interfaces/HH.Lib.Network/Server/nextConnectionAsync/>)\
  Retrieve the next pending connection as a disposable
+ [` ->stopListening(): void `](</hsl/Interfaces/HH.Lib.Network/Server/stopListening/>)\
  Stop listening; open connection are not closed
<!-- HHAPIDOC -->
