
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Starts a local xbox task




``` Hack
function xbox_task_start(
  string $message,
): resource;
```




## Parameters




+ ` string $message ` - A message to send to xbox's message processing
  function.




## Returns




* ` resource ` - - A task handle xbox_task_status() and xbox_task_result()
  can use.
<!-- HHAPIDOC -->
