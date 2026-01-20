
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first value of the given Traversable for which the predicate
returns true, or throws if no such value is found




``` Hack
namespace HH\Lib\C;

function findx<T>(
  Traversable<T> $traversable,
  (function(T): bool) $value_predicate,
): T;
```




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(T): bool) $value_predicate `




## Returns




* ` T `




## Examples




``` basic-usage.hack
$strings = vec["a", "b", "c", "d"];
$predicate_result_1 = C\findx($strings, $x ==> $x == "a");
echo "First predicate: $predicate_result_1\n";
//Output: First predicate: a

$predicate_result_2 = C\findx($strings, $x ==> $x == "z");
//Output: Hit a php exception : exception 'InvariantViolationException' 
//with message 'HH\Lib\C\findx: Couldn't find target value.'
```
<!-- HHAPIDOC -->
