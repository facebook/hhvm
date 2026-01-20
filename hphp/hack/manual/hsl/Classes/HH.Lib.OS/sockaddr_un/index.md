---
title: sockaddr_un
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Address of a UNIX-domain socket




UNIX sockets *may* have a path, which will usually - but not always - exist
on the local filesystem.




See ` man 7 unix ` (Linux) or `` man 6 unix `` (BSD) for details.




## Interface Synopsis




``` Hack
namespace HH\Lib\OS;

final class sockaddr_un extends sockaddr {...}
```




### Public Methods




+ [` ->__construct(?string $path) `](/hsl/Classes/HH.Lib.OS/sockaddr_un/__construct/)
+ [` ->getFamily(): AddressFamily `](/hsl/Classes/HH.Lib.OS/sockaddr_un/getFamily/)
+ [` ->getPath(): ?string `](/hsl/Classes/HH.Lib.OS/sockaddr_un/getPath/)\
  Get the path (if any) of a socket
<!-- HHAPIDOC -->
