
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec sorted by the values of the given Traversable




``` Hack
namespace HH\Lib\Vec;

function sort<Tv>(
  Traversable<Tv> $traversable,
  ?(function(Tv, Tv): num) $comparator = NULL,
): vec<Tv>;
```




If the
optional comparator function isn't provided, the values will be sorted in
ascending order.




To sort by some computable property of each value, see ` Vec\sort_by() `.




Time complexity: O((n log n) * c), where c is the complexity of the
comparator function (which is O(1) if not provided explicitly)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` ?(function(Tv, Tv): num) $comparator = NULL `




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$example_vec = vec[100,2,4,1,6];
$result = Vec\sort($example_vec);
print_r($result);
//result: [1,2,4,6,100]

$example_vec = vec[1,2,3,4,5];
$result = Vec\sort($example_vec);
print_r($result);
//result: [1,2,3,4,5]

$example_vec = vec[0,0,0,0,0];
$result = Vec\sort($example_vec);
print_r($result);
//result: [0,0,0,0,0]
```
<!-- HHAPIDOC -->
