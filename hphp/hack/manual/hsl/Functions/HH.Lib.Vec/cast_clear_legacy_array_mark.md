
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Casts the given traversable to a vec, resetting the legacy array mark
if applicable




``` Hack
namespace HH\Lib\Vec;

function cast_clear_legacy_array_mark<T>(
  Traversable<T> $x,
): vec<T>;
```




## Parameters




+ [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $x ``




## Returns




* ` vec<T> `
<!-- HHAPIDOC -->
