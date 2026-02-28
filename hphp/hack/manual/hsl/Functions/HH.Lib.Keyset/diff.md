
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing only the elements of the first Traversable
that do not appear in any of the other ones




``` Hack
namespace HH\Lib\Keyset;

function diff<Tv1 as arraykey, Tv2 as arraykey>(
  Traversable<Tv1> $first,
  Traversable<Tv2> $second,
  Container<Tv2> ...$rest,
): keyset<Tv1>;
```




Time complexity: O(n + m), where n is size of ` $first ` and m is the combined
size of `` $second `` plus all the ``` ...$rest ```
Space complexity: O(n + m), where n is size of ```` $first ```` and m is the combined
size of ````` $second ````` plus all the `````` ...$rest `````` -- note that this is bigger than
O(n)




## Parameters




+ [` Traversable<Tv1> `](/apis/Interfaces/HH/Traversable/)`` $first ``
+ [` Traversable<Tv2> `](/apis/Interfaces/HH/Traversable/)`` $second ``
+ [` Container<Tv2> `](/apis/Interfaces/HH/Container/)`` ...$rest ``




## Returns




* ` keyset<Tv1> `




## Examples




``` basic-usage.hack
$result = Keyset\diff(keyset[1,2,3,4,5,6], vec[1,2,3]);
print_r($result);
//result: keyset[4,5,6]

$result = Keyset\diff(vec[1,2,3,4,5,6], keyset[]);
print_r($result);
//result: keyset[1,2,3,4,5,6]
```
<!-- HHAPIDOC -->
