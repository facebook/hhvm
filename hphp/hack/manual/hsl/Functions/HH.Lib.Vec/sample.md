
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing an unbiased random sample of up to
` $sample_size ` elements (fewer iff `` $sample_size `` is larger than the size of
``` $traversable ```)




``` Hack
namespace HH\Lib\Vec;

function sample<Tv>(
  Traversable<Tv> $traversable,
  int $sample_size,
): vec<Tv>;
```




Time complexity: O(n), where n is the size of ` $traversable `
Space complexity: O(n), where n is the size of `` $traversable `` -- note that n
may be bigger than ``` $sample_size ```




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` int $sample_size `




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$example_vec = vec[1,2,3,4,5];
$result = Vec\sample($example_vec, 3);
print_r($result);
//result: [2,5,3] OR any 3 elements

$result = Vec\sample($example_vec, 0);
print_r($result);
//result: []

$result = Vec\sample($example_vec, 5);
print_r($result);
//result: [4,5,2,1,3]; All 5 elements
```
<!-- HHAPIDOC -->
