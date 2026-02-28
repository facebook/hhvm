
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict with mapped keys and values




``` Hack
namespace HH\Lib\Dict;

function pull<Tk as arraykey, Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
  (function(Tv1): Tk) $key_func,
): dict<Tk, Tv2>;
```




+ values are the result of calling ` $value_func ` on the original value
+ keys are the result of calling ` $key_func ` on the original value.
  In the case of duplicate keys, later values will overwrite the previous ones.




Time complexity: O(n * (f1 + f2), where f1 is the complexity of ` $value_func `
and f2 is the complexity of `` $key_func ``
Space complexity: O(n)




## Parameters




* [` Traversable<Tv1> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
* ` (function(Tv1): Tv2) $value_func `
* ` (function(Tv1): Tk) $key_func `




## Returns




- ` dict<Tk, Tv2> `
<!-- HHAPIDOC -->
