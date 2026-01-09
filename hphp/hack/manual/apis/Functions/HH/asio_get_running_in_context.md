
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get currently running wait handle in a context specified by its index




``` Hack
namespace HH;

function asio_get_running_in_context(
  int $ctx_idx,
): ResumableWaitHandle<mixed>;
```




## Parameters




+ ` int $ctx_idx `




## Returns




* ` ResumableWaitHandle<mixed> `
<!-- HHAPIDOC -->
