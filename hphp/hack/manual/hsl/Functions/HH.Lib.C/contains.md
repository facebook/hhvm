
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns true if the given Traversable contains the value




``` Hack
namespace HH\Lib\C;

function contains<T1, T2>(
  Traversable<T1> $traversable,
  T2 $value,
): bool;
```




Strict equality is
used.




Time complexity: O(n) (O(1) for keysets)
Space complexity: O(1)




## Parameters




+ [` Traversable<T1> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` T2 $value `




## Returns




* ` bool `




## Examples




``` basic-usage.hack
$strings = vec["a", "b", "c", "d"];
$contains_result_1 = C\contains($strings, "a");
echo "First contains result: $contains_result_1\n";
//Output: First contains result: true

$contains_result_2 = C\contains($strings, "z");
echo "Second contains result: $contains_result_2\n";
//Output: Second contains result: false

$empty_strings = vec[];
$contains_result_3 = C\contains($empty_strings, "a");
echo "Third contains result: $contains_result_3\n";
//Output: Third contains result: false
```
<!-- HHAPIDOC -->
