
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing only the elements of the first Traversable
that appear in all the other ones




``` Hack
namespace HH\Lib\Keyset;

function intersect<Tv as arraykey>(
  Traversable<Tv> $first,
  Traversable<Tv> $second,
  Container<Tv> ...$rest,
): keyset<Tv>;
```




Time complexity: O(n + m), where n is size of ` $first ` and m is the combined
size of `` $second `` plus all the ``` ...$rest ```
Space complexity: O(n), where n is size of ```` $first ````




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $second ``
+ [` Container<Tv> `](/apis/Interfaces/HH/Container/)`` ...$rest ``




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$example_set1 = keyset[1,2,3,4,5];
$example_set2 = keyset[1,3];

$result = Keyset\intersect($example_set1, $example_set2);
print_r($result);
//result: keyset[1,3]

$example_vec3 = keyset[6,7];
$result = Keyset\intersect($example_set1, $example_vec3);
print_r($result);
//result: keyset[]
```
<!-- HHAPIDOC -->
