
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This function uncompress a compressed string




``` Hack
function snappy_uncompress(
  string $data,
): mixed;
```




## Parameters




+ ` string $data ` - The data compressed by snappy_compress()




## Returns




* ` string ` - - The decompressed string or FALSE if an error occurred.
<!-- HHAPIDOC -->
