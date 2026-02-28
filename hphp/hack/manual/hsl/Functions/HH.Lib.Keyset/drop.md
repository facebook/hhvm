
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing all except the first ` $n ` elements of
the given Traversable




``` Hack
namespace HH\Lib\Keyset;

function drop<Tv as arraykey>(
  Traversable<Tv> $traversable,
  int $n,
): keyset<Tv>;
```




To take only the first ` $n ` elements, see `` Keyset\take() ``.




Time complexity: O(n), where n is the size of ` $traversable `
Space complexity: O(n), where n is the size of `` $traversable ``




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` int $n `




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$result = Keyset\drop(keyset[1,2,3,4,5,6], 3);
print_r($result);
//result: keyset[4,5,6]

$result = Keyset\drop(vec[1,2,3,4,5,6], 0);
print_r($result);
//result: keyset[1,2,3,4,5,6]
```
<!-- HHAPIDOC -->
