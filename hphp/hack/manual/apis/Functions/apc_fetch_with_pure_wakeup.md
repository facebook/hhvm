
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Similar to apc_fetch but requires that any deserialized objects' __wakeup
methods can be called from a pure context




``` Hack
function apc_fetch_with_pure_wakeup(
  mixed $key,
  inout mixed $success,
): mixed;
```




## Parameters




+ ` mixed $key `
+ ` inout mixed $success `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
