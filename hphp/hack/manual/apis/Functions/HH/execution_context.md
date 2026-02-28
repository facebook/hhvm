
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a description of the context in which the request is executing




``` Hack
namespace HH;

function execution_context(): string;
```




## Returns




+ ` string ` - - If the request was initiated via the proxygen, xbox,
  pagelet, fastcgi, or replay servers those values are returned. In client
  mode the string cli is returned, when executing in client mode on a server
  (via the unix socket interface) clisrv is returned. On the server with an
  unknown context the string "worker" is returned indicating the job was run
  on an unnamed JobQueue within the server.
<!-- HHAPIDOC -->
