
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

This function compresses the given string using the lz4lib data format, which
is primarily used for compressing and uncompressing memcache values




``` Hack
function lz4_compress(
  string $uncompressed,
  bool $high = false,
): mixed;
```




## Parameters




+ ` string $uncompressed ` - The uncompressed data
+ ` bool $high = false `




## Returns




* ` string ` - - The compressed data, or FALSE on error
<!-- HHAPIDOC -->
