
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Awaitable version of stream_select()







``` Hack
function stream_await(
  resource $fp,
  int $events,
  float $timeout = 0,
): Awaitable<int>;
```




## Parameters




+ ` resource $fp ` - Stream resource, must be backed by a file descriptor
  such as a normal file, socket, tempfile, or stdio.
  Does not work with memory streams or user streams.
+ ` int $events ` - Mix of STREAM_AWAIT_READ and/or STREAM_AWAIT_WRITE
+ ` float $timeout = 0 ` - Timeout in seconds




## Returns




* ` int ` - - Result code
  STREAM_AWAIT_CLOSED: Stream is closed
  STREAM_AWAIT_READY: Activity on the provided stream
  STREAM_AWAIT_TIMEOUT: No activity (timeout occured)
  STREAM_AWAIT_ERROR: Error
<!-- HHAPIDOC -->
