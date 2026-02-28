
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_murmurhash







``` Hack
function hphp_murmurhash(
  string $key,
  int $len,
  int $seed,
): int;
```




## Parameters




+ ` string $key ` - The key to hash
+ ` int $len ` - Number of bytes to use from the key
+ ` int $seed ` - The seed to use for hashing




## Returns




* ` int ` - The Int64 hash of the first len input characters
<!-- HHAPIDOC -->
