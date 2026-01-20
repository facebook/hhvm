
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing only non-null values of the given
Traversable




``` Hack
namespace HH\Lib\Vec;

function filter_nulls<Tv>(
  Traversable<?Tv> $traversable,
): vec<Tv>;
```




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<?Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




* ` vec<Tv> `
<!-- HHAPIDOC -->
