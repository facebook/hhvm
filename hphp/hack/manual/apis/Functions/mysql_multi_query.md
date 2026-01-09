
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

mysql_multi_query() executes one or more queries separated by a ; to the
currently active database on the server that's associated with the specified
link_identifier




``` Hack
function mysql_multi_query(
  string $query,
  resource $link_identifier = NULL,
): mixed;
```




## Parameters




+ ` string $query ` - An SQL query
  The query string should not end with a
  semicolon. Data inside the query should be
  properly escaped.
+ ` resource $link_identifier = NULL ` - The MySQL connection. If the link
  identifier is not specified, the last link
  opened by mysql_connect() is assumed. If
  no such link is found, it will try to
  create one as if mysql_connect() was
  called with no arguments. If no connection
  is found or established, an E_WARNING
  level error is generated.




## Returns




* ` mixed ` - - This is a fb specific query so behaviour is a little random
  at the moment.
<!-- HHAPIDOC -->
