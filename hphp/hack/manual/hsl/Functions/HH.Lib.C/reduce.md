
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reduces the given Traversable into a single value by applying an accumulator
function against an intermediate result and each value




``` Hack
namespace HH\Lib\C;

function reduce<Tv, Ta>(
  Traversable<Tv> $traversable,
  (function(Ta, Tv): Ta) $accumulator,
  Ta $initial,
): Ta;
```




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Ta, Tv): Ta) $accumulator `
+ ` Ta $initial `




## Returns




* ` Ta `




## Examples




``` basic-usage.hack
$values = vec[1,2,3,4,5];
$reduce_result = C\reduce($values, ($total, $value) ==> $total + $value, 0);
echo "Reduce result: $reduce_result\n";
//Output: Reduce result: 15
```
<!-- HHAPIDOC -->
