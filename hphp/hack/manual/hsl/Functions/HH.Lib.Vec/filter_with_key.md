
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing only the values for which the given predicate
returns ` true `




``` Hack
namespace HH\Lib\Vec;

function filter_with_key<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tk, Tv): bool) $predicate,
): vec<Tv>;
```




If you don't need access to the key, see ` Vec\filter() `.




Time complexity: O(n * p), where p is the complexity of ` $predicate `
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` (function(Tk, Tv): bool) $predicate `




## Returns




* ` vec<Tv> `
<!-- HHAPIDOC -->
