
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing all except the first ` $n ` elements of the
given Traversable




``` Hack
namespace HH\Lib\Vec;

function drop<Tv>(
  Traversable<Tv> $traversable,
  int $n,
): vec<Tv>;
```




To take only the first ` $n ` elements, see `` Vec\take() ``.




Time complexity: O(n), where n is the size of ` $traversable `
Space complexity: O(n), where n is the size of `` $traversable ``




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` int $n `




## Returns




* ` vec<Tv> `




## Examples




``` basic-usage.hack
$example_vec = vec[1,2,3,4,5];
$result = Vec\drop($example_vec, 3);
print_r($result);
//result:  [4,5]

$result = Vec\drop($example_vec, 0);
print_r($result);
//result: [1,2,3,4,5] 
```
<!-- HHAPIDOC -->
