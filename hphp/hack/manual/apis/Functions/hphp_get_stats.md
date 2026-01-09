
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks current value of a server stats




``` Hack
function hphp_get_stats(
  string $name,
): int;
```




## Parameters




+ ` string $name ` - Name of the entry.




## Returns




* ` int ` - - Currently accumulated count.
<!-- HHAPIDOC -->
