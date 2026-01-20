
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset sorted by the values of the given Traversable




``` Hack
namespace HH\Lib\Keyset;

function sort<Tv as arraykey>(
  Traversable<Tv> $traversable,
  ?(function(Tv, Tv): num) $comparator = NULL,
): keyset<Tv>;
```




If the
optional comparator function isn't provided, the values will be sorted in
ascending order.




Time complexity: O((n log n) * c), where c is the complexity of the
comparator function (which is O(1) if not explicitly provided)
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` ?(function(Tv, Tv): num) $comparator = NULL `




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$result = Keyset\sort(keyset[1,200, 5]);
print_r($result);
//result: keyset[1,5,200]

$result = Keyset\sort(keyset[100,2000,3,4]);
print_r($result);
//result: keyset[3,4,100,2000]
```
<!-- HHAPIDOC -->
