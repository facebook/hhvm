
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the number of elements in the given Container




``` Hack
namespace HH\Lib\C;

function count(
  Container<mixed> $container,
): int;
```




Time complexity: O(1)
Space complexity: O(1)




## Parameters




+ [` Container<mixed> `](/apis/Interfaces/HH/Container/)`` $container ``




## Returns




* ` int `




## Examples




``` basic-usage.hack
$strings = vec["a", "b", "c", "d", "e"];
$count_result_1 = C\count($strings);
echo "First count result: $count_result_1\n";
//Output: First count result: 5

$empty_strings = vec[];
$count_result_2 = C\count($empty_strings);
echo "Second count result: $count_result_2\n";
//Output: Second count result: 0
```
<!-- HHAPIDOC -->
