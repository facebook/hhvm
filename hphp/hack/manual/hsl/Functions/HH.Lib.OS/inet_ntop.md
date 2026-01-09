
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Convert an INET or INET6 address to presentation format




``` Hack
namespace HH\Lib\OS;

function inet_ntop(
  AddressFamily $af,
  dynamic $addr,
): string;
```




See ` man inet_ntop `




Fails with:

+ ` EAFNOSUPPORT ` if the address family is not supported
+ ` EINVAL ` if the address is the wrong type for the family




## Parameters




* ` AddressFamily $af `
* ` dynamic $addr `




## Returns




- ` string `
<!-- HHAPIDOC -->
