
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Flushes the current buffer




``` Hack
public function flush(
  bool $empty = true,
): mixed;
```




## Parameters




+ ` bool $empty = true ` - Whether to empty the buffer or no. Default is TRUE.




## Returns




* ` mixed ` - - If you opened the writer in memory, this function returns
  the generated XML buffer, Else, if using URI, this function will write the
  buffer and return the number of written bytes.
<!-- HHAPIDOC -->
