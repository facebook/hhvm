
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks an xbox task's status




``` Hack
function xbox_task_status(
  resource $task,
): bool;
```




## Parameters




+ ` resource $task ` - The xbox task object created by xbox_task_start().




## Returns




* ` bool ` - - TRUE if finished, FALSE otherwise.
<!-- HHAPIDOC -->
