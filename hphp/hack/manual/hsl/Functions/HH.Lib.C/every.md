
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns true if the given predicate returns true for every element of the
given Traversable




``` Hack
namespace HH\Lib\C;

function every<T>(
  Traversable<T> $traversable,
  ?(function(T): bool) $predicate = NULL,
): bool;
```




If no predicate is provided, it defaults to casting the
element to bool. If the Traversable is empty, returns true.




If you're looking for ` C\all `, this is it.




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` ?(function(T): bool) $predicate = NULL `




## Returns




* ` bool `




## Examples




``` basic-usage.hack
$strings = vec["a", "b", "c", "d"];
$predicate_result_1 = C\every($strings, $x ==> $x != "z");
echo "First predicate: $predicate_result_1\n";
//Output: First predicate: true
$predicate_result_2 = C\every($strings, $x ==> $x == "a");
echo "Second predicate: $predicate_result_2\n";
//Output: Second predicate: false
$predicate_result_3 = C\every($strings);
echo "Third predicate: $predicate_result_3\n";
//Output: Third predicate: true
$empty_strings = vec[];
$predicate_result_4 = C\every($empty_strings, $x ==> $x == "a");
echo "Fourth predicate: $predicate_result_4\n";
//Output: Fourth predicate: true
$predicate_result_5 = C\every($empty_strings);
echo "Fifth predicate: $predicate_result_5\n";
//Output: Fifth predicate: true
```
<!-- HHAPIDOC -->
