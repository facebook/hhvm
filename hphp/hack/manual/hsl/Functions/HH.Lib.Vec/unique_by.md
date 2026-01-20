
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing each element of the given Traversable exactly
once, where uniqueness is determined by calling the given scalar function on
the values




``` Hack
namespace HH\Lib\Vec;

function unique_by<Tv, Ts as arraykey>(
  Traversable<Tv> $traversable,
  (function(Tv): Ts) $scalar_func,
): vec<Tv>;
```




In case of duplicate scalar keys, later values will overwrite
previous ones.




For arraykey elements, see ` Vec\unique() `.




Time complexity: O(n * s), where s is the complexity of ` $scalar_func `
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv): Ts) $scalar_func `




## Returns




* ` vec<Tv> `
<!-- HHAPIDOC -->
