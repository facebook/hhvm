
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset where each value is the result of calling the given
function on the original value




``` Hack
namespace HH\Lib\Keyset;

function map<Tv1, Tv2 as arraykey>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Tv2) $value_func,
): keyset<Tv2>;
```




Time complexity: O(n * f), where f is the complexity of ` $value_func `
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv1> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv1): Tv2) $value_func `




## Returns




* ` keyset<Tv2> `




## Examples




``` basic-usage.hack
$result = Keyset\map(keyset[1,2,3,4], $val ==> $val*2);
print_r($result);
//result: keyset[2,4,6,8]

$result = Keyset\map(keyset[1,2,3,4], $val ==> $val);
print_r($result);
//result: keyset[1,2,3,4]
```
<!-- HHAPIDOC -->
