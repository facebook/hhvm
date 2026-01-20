
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Write data, waiting if necessary




``` Hack
public function writeAllowPartialSuccessAsync(
  string $bytes,
  ?int $timeout_ns = NULL,
): Awaitable<int>;
```




A wrapper around ` write() ` that will wait if `` write() `` would throw
an [` OS\BlockingIOException `](/hsl/Classes/HH.Lib.OS/BlockingIOException/)




It is possible for the write to *partially* succeed - check the return
value and call again if needed.




## Parameters




+ ` string $bytes `
+ ` ?int $timeout_ns = NULL `




## Returns




* ` the ` - number of bytes written, which may be less than the length of
  input string.
<!-- HHAPIDOC -->
