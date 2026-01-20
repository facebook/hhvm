
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset where each value is the result of calling the given
function on the original key and value




``` Hack
namespace HH\Lib\Keyset;

function map_with_key<Tk, Tv1, Tv2 as arraykey>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tk, Tv1): Tv2) $value_func,
): keyset<Tv2>;
```




Time complexity: O(n * f), where f is the complexity of ` $value_func `
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv1> $traversable ``
+ ` (function(Tk, Tv1): Tv2) $value_func `




## Returns




* ` keyset<Tv2> `




## Examples




``` basic-usage.hack
$result = Keyset\map_with_key(dict[1 => 2, 2 => 4, 3 => 5], ($key, $val) ==> $val*$key);
print_r($result);
//result: keyset[2,8,15]

$result = Keyset\map_with_key(dict[1 => 2, 2 => 4], ($key, $val) ==> $key);
print_r($result);
//result: keyset[1,2]
```
<!-- HHAPIDOC -->
