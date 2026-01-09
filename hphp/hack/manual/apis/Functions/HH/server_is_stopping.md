
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Whether the server is going to stop soon




``` Hack
namespace HH;

function server_is_stopping(): bool;
```




## Returns




+ ` bool ` - - True if server is going to stop soon, False if
  server is not running, or is running without a schedule to stop.
<!-- HHAPIDOC -->
