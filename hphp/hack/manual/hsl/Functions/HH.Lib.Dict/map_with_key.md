
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each value is the result of calling the given
function on the original value and key




``` Hack
namespace HH\Lib\Dict;

function map_with_key<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tk, Tv1): Tv2) $value_func,
): dict<Tk, Tv2>;
```




Time complexity: O(n * f), where f is the complexity of ` $value_func `
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv1> $traversable ``
+ ` (function(Tk, Tv1): Tv2) $value_func `




## Returns




* ` dict<Tk, Tv2> `
<!-- HHAPIDOC -->
