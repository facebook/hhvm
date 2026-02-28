
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict with mapped keys and values




``` Hack
namespace HH\Lib\Dict;

function pull_with_key<Tk1, Tk2 as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk1, Tv1> $traversable,
  (function(Tk1, Tv1): Tv2) $value_func,
  (function(Tk1, Tv1): Tk2) $key_func,
): dict<Tk2, Tv2>;
```




+ values are the result of calling ` $value_func ` on the original value/key
+ keys are the result of calling ` $key_func ` on the original value/key.
  In the case of duplicate keys, later values will overwrite the previous ones.




Time complexity: O(n * (f1 + f2), where f1 is the complexity of ` $value_func `
and f2 is the complexity of `` $key_func ``
Space complexity: O(n)




## Parameters




* [` KeyedTraversable<Tk1, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv1> $traversable ``
* ` (function(Tk1, Tv1): Tv2) $value_func `
* ` (function(Tk1, Tv1): Tk2) $key_func `




## Returns




- ` dict<Tk2, Tv2> `
<!-- HHAPIDOC -->
