
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing only the elements of the first Traversable that
appear in all the other ones




``` Hack
namespace HH\Lib\Vec;

function intersect<Tv as arraykey>(
  Traversable<Tv> $first,
  Traversable<Tv> $second,
  Container<Tv> ...$rest,
): vec<Tv>;
```




Duplicate values are preserved.




Time complexity: O(n + m), where n is size of ` $first ` and m is the combined
size of `` $second `` plus all the ``` ...$rest ```
Space complexity: O(n), where n is size of ```` $first ````




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $second ``
+ [` Container<Tv> `](/apis/Interfaces/HH/Container/)`` ...$rest ``




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$example_vec1 = vec[1,2,3,4,5];
$example_vec2 = vec[1,3];

$result = Vec\intersect($example_vec1, $example_vec2);
print_r($result);
//result: [1,3]

$example_vec3 = vec[6,7];
$result = Vec\intersect($example_vec1, $example_vec3);
print_r($result);
//result: []
```
<!-- HHAPIDOC -->
