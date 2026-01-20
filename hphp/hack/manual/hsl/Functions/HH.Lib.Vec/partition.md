
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a 2-tuple containing vecs for which the given predicate returned
` true ` and `` false ``, respectively




``` Hack
namespace HH\Lib\Vec;

function partition<Tv>(
  Traversable<Tv> $traversable,
  (function(Tv): bool) $predicate,
): (vec<Tv>, vec<Tv>);
```




Time complexity: O(n * p), where p is the complexity of ` $predicate `
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv): bool) $predicate `




## Returns




* ` (vec<Tv>, vec<Tv>) `




## Examples




``` basic-usage.hack
$example_vec = vec[1,2,3,4,5];
$result = Vec\partition($example_vec, $val ==> $val%2 == 0);
print_r($result);
// result: [[2,4], [1,3,5]]

$result = Vec\partition($example_vec, $val ==> $val == $val);
print_r($result);
//result: [[1,2,3,4,5], []]

$result = Vec\partition($example_vec, $val ==> $val != 0);
print_r($result);
//result: [[1,2,3,4,5], []]
```
<!-- HHAPIDOC -->
