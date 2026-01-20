
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing only non-null values of the given
Traversable




``` Hack
namespace HH\Lib\Keyset;

function filter_nulls<Tv as arraykey>(
  Traversable<?Tv> $traversable,
): keyset<Tv>;
```




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` Traversable<?Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$result = Keyset\filter_nulls(vec<int>[1,2,3,4,null, null]);
print_r($result);
//result: keyset[1,2,3,4]

$result = Keyset\filter_nulls(vec[1,2,3,4]);
print_r($result);
//result: keyset[1,2,3,4]

$result = Keyset\filter_nulls(vec[]);
print_r($result);
//result: keyset[]
```
<!-- HHAPIDOC -->
