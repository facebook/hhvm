
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a vec containing the given Traversable split into chunks of the
given size




``` Hack
namespace HH\Lib\Keyset;

function chunk<Tv as arraykey>(
  Traversable<Tv> $traversable,
  int $size,
): vec<keyset<Tv>>;
```




If the given Traversable doesn't divide evenly, the final chunk will be
smaller than the specified size. If there are duplicate values in the
Traversable, some chunks may be smaller than the specified size.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` int $size `




## Returns




* ` vec<keyset<Tv>> `




## Examples




``` basic-usage.hack
$result = Keyset\chunk(vec[1,2,3,4,5,6],3);
print_r($result);
//result: [keyset[1,2,3], keyset[4,5,6]]

$result = Keyset\chunk(vec[1,2,2,3,4,5],3);
print_r($result);
//result: [keyset[1,2], keyset[3,4,5]]
```
<!-- HHAPIDOC -->
