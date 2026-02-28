
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec sorted by some scalar property of each value of the given
Traversable, which is computed by the given function




``` Hack
namespace HH\Lib\Vec;

function sort_by<Tv, Ts>(
  Traversable<Tv> $traversable,
  (function(Tv): Ts) $scalar_func,
  ?(function(Ts, Ts): num) $comparator = NULL,
): vec<Tv>;
```




If the optional
comparator function isn't provided, the values will be sorted in ascending
order of scalar key.




To sort by the values of the Traversable, see ` Vec\sort() `.




Time complexity: O((n log n) * c + n * s), where c is the complexity of the
comparator function (which is O(1) if not provided explicitly) and s is the
complexity of the scalar function
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv): Ts) $scalar_func `
+ ` ?(function(Ts, Ts): num) $comparator = NULL `




## Returns




* ` vec<Tv> `
<!-- HHAPIDOC -->
