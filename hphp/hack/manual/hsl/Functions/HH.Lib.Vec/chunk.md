
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a vec containing the original vec split into chunks of the given
size




``` Hack
namespace HH\Lib\Vec;

function chunk<Tv>(
  Traversable<Tv> $traversable,
  int $size,
): vec<vec<Tv>>;
```




If the original vec doesn't divide evenly, the final chunk will be
smaller.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` int $size `




## Returns




* ` vec<vec<Tv>> `




## Examples




``` basic-usage.hack
$example_vec = vec[1,2,3,4,5];
$chunks = Vec\chunk($example_vec, 3);
print_r($chunks);
// result [[1,2,3], [4,5]]
$chunks = Vec\chunk($example_vec, 5);
print_r($chunks);
// result [1,2,3,4,5]
```
<!-- HHAPIDOC -->
