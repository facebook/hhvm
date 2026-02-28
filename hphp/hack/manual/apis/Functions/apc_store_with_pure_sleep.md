
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Similar to apc_store but requires that any serialized objects' __sleep
methods can be called from a pure context




``` Hack
function apc_store_with_pure_sleep(
  mixed $key_or_array,
  mixed $var = NULL,
  int $ttl = 0,
  int $bump_ttl = 0,
): mixed;
```




## Parameters




+ ` mixed $key_or_array `
+ ` mixed $var = NULL `
+ ` int $ttl = 0 `
+ ` int $bump_ttl = 0 `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
