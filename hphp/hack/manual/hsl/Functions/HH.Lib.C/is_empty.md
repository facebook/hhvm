
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the given Container is empty




``` Hack
namespace HH\Lib\C;

function is_empty<T>(
  Container<T> $container,
): bool;
```




Time complexity: O(1)
Space complexity: O(1)




## Parameters




+ [` Container<T> `](/apis/Interfaces/HH/Container/)`` $container ``




## Returns




* ` bool `




## Examples




``` basie-usage.hack
$strings = vec["a", "b", "c", "d", "e"];
$is_empty_result_1 = C\is_empty($strings);
echo "First is_empty result: $is_empty_result_1\n";
//Output: First is_empty result: false

$empty_strings = vec[];
$is_empty_result_2 = C\is_empty($empty_strings);
echo "Second is_empty result: $is_empty_result_2\n";
//Output: Second is_empty result: true
```
<!-- HHAPIDOC -->
