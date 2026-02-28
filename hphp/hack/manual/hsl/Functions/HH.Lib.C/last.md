
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last element of the given Traversable, or null if the
Traversable is empty




``` Hack
namespace HH\Lib\C;

function last<T>(
  Traversable<T> $traversable,
): ?T;
```




+ For non-empty Traversables, see ` C\lastx `.
+ For single-element Traversables, see ` C\onlyx `.




Time complexity: O(1) if ` $traversable ` is a [` Container `](/apis/Interfaces/HH/Container/), O(n) otherwise.
Space complexity: O(1)




## Parameters




* [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




- ` ?T `




## Examples




``` basic-usage.hack
$strings = vec["a", "b", "c"];
$last_string = C\last($strings);
echo "Last string in traversable: $last_string \n";
//Output: Last string in traversable: c

$empty_traversable = vec[];
$last_element = C\last($empty_traversable);
$last_element_as_string = $last_element ?? "null";
echo "Last element in empty traversable: $last_element_as_string";
//Output: Last element in empty traversable: null
```
<!-- HHAPIDOC -->
