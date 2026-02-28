
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Tallies a number for server stats




``` Hack
function hphp_stats(
  string $name,
  int $value,
): void;
```




## Parameters




+ ` string $name ` - Name of the entry. This name can then be used with
  admin commands to retrieve stats while server is running.
+ ` int $value ` - An integer to add up.




## Returns




* ` void `
<!-- HHAPIDOC -->
