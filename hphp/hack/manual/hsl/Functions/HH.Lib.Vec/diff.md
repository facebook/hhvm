
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing only the elements of the first Traversable that
do not appear in any of the other ones




``` Hack
namespace HH\Lib\Vec;

function diff<Tv1 as arraykey, Tv2 as arraykey>(
  Traversable<Tv1> $first,
  Traversable<Tv2> $second,
  Container<Tv2> ...$rest,
): vec<Tv1>;
```




For vecs that contain non-arraykey elements, see ` Vec\diff_by() `.




Time complexity: O(n + m), where n is size of ` $first ` and m is the combined
size of `` $second `` plus all the ``` ...$rest ```
Space complexity: O(n + m), where n is size of ```` $first ```` and m is the combined
size of ````` $second ````` plus all the `````` ...$rest `````` -- note that this is bigger than
O(n)




## Parameters




+ [` Traversable<Tv1> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Traversable<Tv2> `](/apis/Interfaces/HH/Traversable/)`` $second ``
+ [` Container<Tv2> `](/apis/Interfaces/HH/Container/)`` ...$rest ``




## Returns




* ` vec<Tv1> `




## Examples




``` basic-usage.hack
$example_vec1 = vec[1,2,3,4,5];
$example_vec2 = vec[1,3];

$diffed_vec = Vec\diff($example_vec1, $example_vec2);
print_r($diffed_vec);
// result: [2,4,5]

$example_vec3 = vec[6,7];

$diffed_vec = Vec\diff($example_vec1, $example_vec3);
print_r($diffed_vec);
// result: [1,2,3,4,5] 
```
<!-- HHAPIDOC -->
