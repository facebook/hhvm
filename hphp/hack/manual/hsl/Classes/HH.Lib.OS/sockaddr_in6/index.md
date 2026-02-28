---
title: sockaddr_in6
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Address of an INET6 (IPv6) socket




See ` man 7 ip6 ` (Linux) or `` man 4 inet6 `` (BSD) for details.




## Interface Synopsis




``` Hack
namespace HH\Lib\OS;

final class sockaddr_in6 extends sockaddr {...}
```




### Public Methods




+ [` ->__construct(int $port, int $flowInfo, in6_addr $address, int $scopeID) `](/hsl/Classes/HH.Lib.OS/sockaddr_in6/__construct/)\
  Construct an instance
+ [` ->__debugInfo(): darray<string, mixed> `](/hsl/Classes/HH.Lib.OS/sockaddr_in6/__debugInfo/)
+ [` ->getAddress(): in6_addr `](/hsl/Classes/HH.Lib.OS/sockaddr_in6/getAddress/)
+ [` ->getFamily(): AddressFamily `](/hsl/Classes/HH.Lib.OS/sockaddr_in6/getFamily/)
+ [` ->getFlowInfo(): int `](/hsl/Classes/HH.Lib.OS/sockaddr_in6/getFlowInfo/)\
  Get the flow ID
+ [` ->getPort(): int `](/hsl/Classes/HH.Lib.OS/sockaddr_in6/getPort/)\
  Get the port, in host byte order
+ [` ->getScopeID(): int `](/hsl/Classes/HH.Lib.OS/sockaddr_in6/getScopeID/)\
  Get the scope ID
<!-- HHAPIDOC -->
