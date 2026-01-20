
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reduces the given KeyedTraversable into a single value by
applying an accumulator function against an intermediate result
and each key/value




``` Hack
namespace HH\Lib\C;

function reduce_with_key<Tk, Tv, Ta>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Ta, Tk, Tv): Ta) $accumulator,
  Ta $initial,
): Ta;
```




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` (function(Ta, Tk, Tv): Ta) $accumulator `
+ ` Ta $initial `




## Returns




* ` Ta `




## Examples




``` basic-usage.hack
$dict = dict["a" => 1, "b" => 2, "c" => 3];
$list = vec["a","b"];
$reduce_with_key_result = C\reduce_with_key($dict, ($a, $k, $v) ==> $a + (C\contains($list, $k) ? 0 : $v), 0);
echo "Reduce with key result: $reduce_with_key_result\n";
//Output: Reduce with key result: 3
```
<!-- HHAPIDOC -->
