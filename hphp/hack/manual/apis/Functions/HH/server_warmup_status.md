
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a (bad) description of the warmup status of the server, based on
request-specific state




``` Hack
namespace HH;

function server_warmup_status(): string;
```




## Returns




+ ` string ` - - If the server appears to be warmed up, returns the empty
  string. Otherwise, returns a human-readable description of why the server is
  not warmed up. Note that this function checks a series of heuristics rather
  than anything definitive; returning '' for one request does not guarantee
  the same result for subsequent requests.
<!-- HHAPIDOC -->
