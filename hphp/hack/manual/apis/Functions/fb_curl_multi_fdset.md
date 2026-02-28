
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

extracts file descriptor information from a multi handle




``` Hack
function fb_curl_multi_fdset(
  resource $mh,
  inout array& $read_fd_set,
  inout array& $write_fd_set,
  inout array& $exc_fd_set,
  inout int& $max_fd,
): mixed;
```




## Parameters




+ ` resource $mh ` - A cURL multi handle returned by
  curl_multi_init().
+ ` inout array& $read_fd_set ` - read set
+ ` inout array& $write_fd_set ` - write set
+ ` inout array& $exc_fd_set ` - exception set
+ ` inout int& $max_fd ` - If no file descriptors are set, $max_fd will
  contain -1. Otherwise it will contain the higher descriptor number.




## Returns




* ` mixed ` - - Returns 0 on success, or one of the CURLM_XXX errors code.
<!-- HHAPIDOC -->
