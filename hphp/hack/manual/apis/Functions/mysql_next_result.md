
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Used with mysql_multi_query() to move the result set on one




``` Hack
function mysql_next_result(
  resource $link_identifier = NULL,
): int;
```




## Parameters




+ ` resource $link_identifier = NULL ` - The MySQL connection. If the link
  identifier is not specified, the last link
  opened by mysql_connect() is assumed. If
  no such link is found, it will try to
  create one as if mysql_connect() was
  called with no arguments. If no connection
  is found or established, an E_WARNING
  level error is generated.




## Returns




* ` int ` - - 0 - Query succeeded, more results coming. -1 - Query succeeded,
  no more results coming. >0 - query failed, value is error code.
<!-- HHAPIDOC -->
