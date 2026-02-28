
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Block and wait until pagelet task finishes or times out




``` Hack
function pagelet_server_task_result(
  resource $task,
  inout mixed $headers,
  inout mixed $code,
  int $timeout_ms = 0,
): string;
```




## Parameters




+ ` resource $task ` - The pagelet task handle returned from
  pagelet_server_task_start().
+ ` inout mixed $headers ` - HTTP response headers.
+ ` inout mixed $code ` - HTTP response code. Set to -1 in the event of a
  timeout.
+ ` int $timeout_ms = 0 ` - How many milliseconds to wait. A timeout of zero
  is interpreted as an infinite timeout.




## Returns




* ` string ` - - HTTP response from the pagelet.
<!-- HHAPIDOC -->
