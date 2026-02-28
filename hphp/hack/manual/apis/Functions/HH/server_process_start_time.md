
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the timestamp when the http server process was started




``` Hack
namespace HH;

function server_process_start_time(): int;
```




## Returns




+ ` int ` - - number of seconds since epoch when process started.  0 if
  server is not started.
<!-- HHAPIDOC -->
