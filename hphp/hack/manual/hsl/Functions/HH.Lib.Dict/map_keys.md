
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each key is the result of calling the given
function on the original key




``` Hack
namespace HH\Lib\Dict;

function map_keys<Tk1, Tk2 as arraykey, Tv>(
  KeyedTraversable<Tk1, Tv> $traversable,
  (function(Tk1): Tk2) $key_func,
): dict<Tk2, Tv>;
```




In the case of duplicate keys, later values
will overwrite the previous ones.




Time complexity: O(n * f), where f is the complexity of ` $key_func `
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk1, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` (function(Tk1): Tk2) $key_func `




## Returns




* ` dict<Tk2, Tv> `
<!-- HHAPIDOC -->
