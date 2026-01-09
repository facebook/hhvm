
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Convert an INET (IPv4) address from network format to presentation
(dotted) format




``` Hack
namespace HH\Lib\OS;

function inet_ntop_inet(
  in_addr $addr,
): string;
```




See ` man inet_ntop `




## Parameters




+ ` in_addr $addr `




## Returns




* ` string `
<!-- HHAPIDOC -->
