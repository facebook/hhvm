
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Sets query timeout for a connection







``` Hack
function mysql_set_timeout(
  int $query_timeout_ms = -1,
  resource $link_identifier = NULL,
): bool;
```




## Parameters




+ ` int $query_timeout_ms = -1 ` - How many milli-seconds to wait for an SQL
  query
+ ` resource $link_identifier = NULL ` - Which connection to set to. If absent,
  default or current connection will be
  applied to.




## Returns




* ` bool `
<!-- HHAPIDOC -->
