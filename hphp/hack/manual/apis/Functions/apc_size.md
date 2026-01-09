
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Find the in-memory size of a key in APC, for debugging purposes




``` Hack
function apc_size(
  string $key,
): ?int;
```




## Parameters




+ ` string $key ` - The key to find the size of.




## Returns




* ` mixed ` - - Returns the current size of a key or null on failure.
<!-- HHAPIDOC -->
