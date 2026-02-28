
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Block and wait for xbox task's result




``` Hack
function xbox_task_result(
  resource $task,
  int $timeout_ms,
  inout mixed $ret,
): int;
```




## Parameters




+ ` resource $task ` - The xbox task object created by xbox_task_start().
+ ` int $timeout_ms ` - How many milli-seconds to wait.
+ ` inout mixed $ret ` - xbox message processing function's return value.




## Returns




* ` int ` - - Response code following HTTP's responses. For example, 200
  for success and 500 for server error.
<!-- HHAPIDOC -->
