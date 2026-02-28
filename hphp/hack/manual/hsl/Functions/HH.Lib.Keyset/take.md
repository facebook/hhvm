
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing the first ` $n ` elements of the given
Traversable




``` Hack
namespace HH\Lib\Keyset;

function take<Tv as arraykey>(
  Traversable<Tv> $traversable,
  int $n,
): keyset<Tv>;
```




If there are duplicate values in the Traversable, the keyset may be shorter
than the specified length.




To drop the first ` $n ` elements, see `` Keyset\drop() ``.




Time complexity: O(n), where n is ` $n `
Space complexity: O(n), where n is `` $n ``




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` int $n `




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$result = Keyset\take(keyset[1,200, 5], 2);
print_r($result);
//result: keyset[1,200]

$result = Keyset\take(keyset[100,2000,3,4], 0);
print_r($result);
//result: keyset[]
```
<!-- HHAPIDOC -->
