
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the time that the http server has been accepting connections




``` Hack
namespace HH;

function server_uptime(): int;
```




## Returns




+ ` int ` - - number of seconds the server has been running.  -1 if
  server is not started.
<!-- HHAPIDOC -->
