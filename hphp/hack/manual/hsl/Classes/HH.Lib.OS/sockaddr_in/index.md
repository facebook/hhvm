---
title: sockaddr_in
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Address of an INET (IPv4) socket




See ` man 7 ip ` (Linux) or `` man 4 inet `` (BSD) for details.




## Interface Synopsis




``` Hack
namespace HH\Lib\OS;

final class sockaddr_in extends sockaddr {...}
```




### Public Methods




+ [` ->__construct(int $port, in_addr $address) `](/hsl/Classes/HH.Lib.OS/sockaddr_in/__construct/)\
  Construct a `` sockaddr_in ``
+ [` ->__debugInfo(): darray<string, mixed> `](/hsl/Classes/HH.Lib.OS/sockaddr_in/__debugInfo/)
+ [` ->getAddress(): in_addr `](/hsl/Classes/HH.Lib.OS/sockaddr_in/getAddress/)\
  Get the IP address, as a 32-bit integer, in host byte order
+ [` ->getFamily(): AddressFamily `](/hsl/Classes/HH.Lib.OS/sockaddr_in/getFamily/)
+ [` ->getPort(): int `](/hsl/Classes/HH.Lib.OS/sockaddr_in/getPort/)\
  Get the port, in host byte order
<!-- HHAPIDOC -->
