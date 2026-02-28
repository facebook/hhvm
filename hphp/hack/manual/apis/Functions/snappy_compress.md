
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This function compress the given string using the Snappy data format




``` Hack
function snappy_compress(
  string $data,
): mixed;
```




For details on the Snappy compression algorithm go to
http://code.google.com/p/snappy/.




## Parameters




+ ` string $data ` - The data to compress




## Returns




* ` string ` - - The compressed string or FALSE if an error occurred.
<!-- HHAPIDOC -->
