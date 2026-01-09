
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Processes a pagelet server request




``` Hack
function pagelet_server_task_start(
  string $url,
  array $headers = dict [
],
  string $post_data = '',
  array $files = dict [
],
  int $timeout_seconds = 0,
): resource;
```




## Parameters




+ ` string $url ` - The URL we're running this pagelet with.
+ ` array $headers = dict [ ] ` - HTTP headers to send to the pagelet.
+ ` string $post_data = '' ` - POST data to send.
+ ` array $files = dict [ ] ` - Array for the pagelet.
+ ` int $timeout_seconds = 0 `




## Returns




* ` resource ` - - An object that can be used with
  pagelet_server_task_status() or pagelet_server_task_result().
<!-- HHAPIDOC -->
