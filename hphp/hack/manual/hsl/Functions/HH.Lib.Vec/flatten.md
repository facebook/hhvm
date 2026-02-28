
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec formed by joining the Traversable elements of the given
Traversable




``` Hack
namespace HH\Lib\Vec;

function flatten<Tv>(
  Traversable<Container<Tv>> $traversables,
): vec<Tv>;
```




For a fixed number of Traversables, see ` Vec\concat() `.




Time complexity: O(n), where n is the combined size of all the
` $traversables `
Space complexity: O(n), where n is the combined size of all the
`` $traversables ``




## Parameters




+ [` Traversable<Container<Tv>> `](/apis/Interfaces/HH/Traversable/)`` $traversables ``




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$example_vec = vec[vec[1,2,3,4,5],vec[98,99]];
$result = Vec\flatten($example_vec);
print_r($result);
//result: [1,2,3,4,5,98,99]

$example_vec = vec[vec[1,2,3,4,5,98,99]];
$result = Vec\flatten($example_vec);
print_r($result);
//result: [1,2,3,4,5,98,99]

$example_vec = vec[];
$result = Vec\flatten($example_vec);
print_r($result);
//result: []
```
<!-- HHAPIDOC -->
