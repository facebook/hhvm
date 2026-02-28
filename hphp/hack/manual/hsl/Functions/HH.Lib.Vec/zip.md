
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a vec where each element is a tuple (pair) that combines, pairwise,
the elements of the two given Traversables




``` Hack
namespace HH\Lib\Vec;

function zip<Tv, Tu>(
  Traversable<Tv> $first,
  Traversable<Tu> $second,
): vec<(Tv, Tu)>;
```




If the Traversables are not of equal length, the result will have
the same number of elements as the shortest Traversable.
Elements of the longer Traversable after the length of the shorter one
will be ignored.




Time complexity: O(min(m, n)), where m is the size of ` $first ` and n is the
size of `` $second ``
Space complexity: O(min(m, n)), where m is the size of ``` $first ``` and n is the
size of ```` $second ````




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $second ``




## Returns




* ` vec<(Tv, Tu)> `




## Examples




``` basic-usage.hack
$example_vec1 = vec[1,2,3,4,5];
$example_vec2 = vec[11,17,23,44,55];

$result = Vec\zip($example_vec1, $example_vec2);
print_r($result);
//result: [[1, 11], [2,17], [3,23], [4, 44], [5,55]]

$example_vec3 = vec[6,7];
$result = Vec\zip($example_vec1, $example_vec3);
print_r($result);
//result: [[1,6], [2,7]]
```
<!-- HHAPIDOC -->
