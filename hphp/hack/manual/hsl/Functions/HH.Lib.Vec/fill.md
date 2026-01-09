
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec of size ` $size ` where all the values are `` $value ``




``` Hack
namespace HH\Lib\Vec;

function fill<Tv>(
  int $size,
  Tv $value,
): vec<Tv>;
```




If you need a range of items not repeats, use ` Vec\range(0, $n - 1) `.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ ` int $size `
+ ` Tv $value `




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$result = Vec\fill(2, 100);
print_r($result);
//result: [100, 100]

$result = Vec\fill(0, 10);
print_r($result);
//result: []

$result = Vec\fill(5, 1);
print_r($result);
//result: [1,1,1,1,1]
```
<!-- HHAPIDOC -->
