
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the health level of the server in the range of 0~100




``` Hack
namespace HH;

function server_health_level(): int;
```




## Returns




+ ` int ` - - 100 if the server is very healthy, and 0 if the
  server should not receive any more request.
<!-- HHAPIDOC -->
