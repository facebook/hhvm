
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first value of the given Traversable for which the predicate
returns true, or null if no such value is found




``` Hack
namespace HH\Lib\C;

function find<T>(
  Traversable<T> $traversable,
  (function(T): bool) $value_predicate,
): ?T;
```




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(T): bool) $value_predicate `




## Returns




* ` ?T `




## Examples




``` basic-usage.hack
$strings = vec["a", "b", "c", "d"];
$predicate_string_1 = C\find($strings, $x ==> $x == "b");
echo "First predicate: $predicate_string_1\n";
//Output: First predicate: b

$predicate_string_2 = C\find($strings, $x ==> $x == "z");
$predicate_string_2_as_string = $predicate_string_2 ?? "null";
echo "Second predicate: $predicate_string_2_as_string\n";
//Output: Second predicate: null

$repeat_strings = vec["a1", "b", "a2", "d"];
$predicate_string_3 = C\find($repeat_strings, $x ==> ($x == "a1" || $x == "a2"));
echo "Repeat_predicate: $predicate_string_3\n";
//Output: Repeat_predicate: a1
```
<!-- HHAPIDOC -->
