
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks finish status of a pagelet task




``` Hack
function pagelet_server_task_status(
  resource $task,
): int;
```




## Parameters




+ ` resource $task ` - The pagelet task handle returned from
  pagelet_server_task_start().




## Returns




* ` int ` - - PAGELET_NOT_READY if there is no data available,
  PAGELET_READY if (partial) data is available from pagelet_server_flush(),
  and PAGELET_DONE if the pagelet request is done.
<!-- HHAPIDOC -->
