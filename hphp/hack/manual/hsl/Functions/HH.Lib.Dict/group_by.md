
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return a dict keyed by the result of calling the giving function, preserving
duplicate values




``` Hack
namespace HH\Lib\Dict;

function group_by<Tk as arraykey, Tv>(
  Traversable<Tv> $values,
  (function(Tv): ?Tk) $key_func,
): dict<Tk, vec<Tv>>;
```




+ keys are the results of the given function called on the given values.
+ values are vecs of original values that all produced the same key.




If a value produces a null key, it's omitted from the result.




Time complexity: O(n * f), where f is the complexity of ` $key_func `
Space complexity: O(n)




## Parameters




* [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $values ``
* ` (function(Tv): ?Tk) $key_func `




## Returns




- ` dict<Tk, vec<Tv>> `




## Examples




Group numbers by their parity (separate even and odd numbers):




``` basic-usage.hack
$numbers = vec[1, 1, 2, 3, 5, 8, 14];
$groups = Dict\group_by($numbers, $value ==> $value % 2);
\print_r($groups);
```
<!-- HHAPIDOC -->
