
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return a map of event_name->EventStats for all events which occurred during
previously completed requests at the time this function was called




``` Hack
namespace HH\rqtrace;

function all_process_stats(): dict<string, dict<string, int>>;
```




## Returns




+ ` dict<string, dict<string, int>> `
<!-- HHAPIDOC -->
