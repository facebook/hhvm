
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the key of the first value of the given KeyedTraversable for which
the predicate returns true, or null if no such value is found




``` Hack
namespace HH\Lib\C;

function find_key<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tv): bool) $value_predicate,
): ?Tk;
```




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` (function(Tv): bool) $value_predicate `




## Returns




* ` ?Tk `




## Examples




``` basic-usage.hack
$dict = dict["key_1" => "a", "key_2" => "b", "key_3" => "c"];
$predicate_result_1 = C\find_key($dict, $x ==> $x == "a");
echo "First predicate: $predicate_result_1\n";
//Output: First predicate: key_1

$predicate_result_2 = C\find_key($dict, $x ==> $x == "z");
$predicate_result_2_as_string = $predicate_result_2 ?? "null";
echo "Second predicate: $predicate_result_2_as_string\n";
//Output: Second predicate: null

$repeat_dict = dict["key_1" => "a", "key_2" => "b", "key_3" => "c", "key_4" => "a"];
$predicate_result_3 = C\find_key($dict, $x ==> $x == "a");
echo "Repeat_predicate: $predicate_result_3\n";
//Output: Repeat_predicate: key_1
```
<!-- HHAPIDOC -->
