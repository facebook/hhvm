
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict mapping each value to the number of times it appears
in the given Traversable




``` Hack
namespace HH\Lib\Dict;

function count_values<Tv as arraykey>(
  Traversable<Tv> $values,
): dict<Tv, int>;
```




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $values ``




## Returns




* ` dict<Tv, int> `




## Examples




``` basic-usage.hack
$result = Dict\count_values(vec[1,2,2,3,3,3]);
print_r($result);
//result: dict[1=>1, 2=>2, 3=>3]

$result = Dict\count_values(vec[0,0,0,0,0]);
print_r($result);
//result: dict[0=>5]
```
<!-- HHAPIDOC -->
