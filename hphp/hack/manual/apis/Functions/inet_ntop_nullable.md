
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Converts a packed internet address to a human readable representation







``` Hack
function inet_ntop_nullable(
  string $in_addr,
): ?string;
```




## Parameters




+ ` string $in_addr ` - A 32bit IPv4, or 128bit IPv6 address.




## Returns




* ` null|string ` - - Returns a string representation of the address.
<!-- HHAPIDOC -->
