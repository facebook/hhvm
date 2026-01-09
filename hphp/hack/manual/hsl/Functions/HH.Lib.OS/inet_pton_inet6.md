
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Convert a presentation-format (colon-separated) INET6 (IPv6) address to
network format




``` Hack
namespace HH\Lib\OS;

function inet_pton_inet6(
  string $addr,
): in6_addr;
```




See ` man inet_pton `.




## Parameters




+ ` string $addr `




## Returns




* ` in6_addr `
<!-- HHAPIDOC -->
