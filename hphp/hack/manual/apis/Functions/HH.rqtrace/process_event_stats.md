
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return stats for all occurences of $event during previously completed
requests when this function was called




``` Hack
namespace HH\rqtrace;

function process_event_stats(
  string $event_name,
): dict<string, int>;
```




## Parameters




+ ` string $event_name `




## Returns




* ` dict<string, int> `
<!-- HHAPIDOC -->
