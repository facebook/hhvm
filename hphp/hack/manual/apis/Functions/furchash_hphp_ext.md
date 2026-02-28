
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

furchash_hphp_ext







``` Hack
function furchash_hphp_ext(
  string $key,
  int $len,
  int $nPart,
): int;
```




## Parameters




+ ` string $key ` - The key to hash
+ ` int $len ` - Number of bytes to use from the hash
+ ` int $nPart `




## Returns




* ` int ` - - A number in the range of 0-(nPart-1)
<!-- HHAPIDOC -->
