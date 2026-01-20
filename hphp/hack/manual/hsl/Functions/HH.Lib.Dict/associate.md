
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each element in ` $keys ` maps to the
corresponding element in `` $values ``




``` Hack
namespace HH\Lib\Dict;

function associate<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  Traversable<Tv> $values,
): dict<Tk, Tv>;
```




Time complexity: O(n) where n is the size of ` $keys ` (which must be the same
as the size of `` $values ``)
Space complexity: O(n) where n is the size of ``` $keys ``` (which must be the same
as the size of ```` $values ````)




## Parameters




+ [` Traversable<Tk> `](/apis/Interfaces/HH/Traversable/)`` $keys ``
+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $values ``




## Returns




* ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
// NOTE: $keys, $values must be the same length
$keys = vec[1,2,3,4,5];
$values = vec[1,4,9,16,25];
$dict = Dict\associate($keys, $values);
\print_r($dict);
//Output: dict[1 => 1, 2 => 4, 3 => 9, 4 => 16, 5 => 25]
```
<!-- HHAPIDOC -->
