
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first element of the given Traversable, or null if the
Traversable is empty




``` Hack
namespace HH\Lib\C;

function first<T>(
  Traversable<T> $traversable,
): ?T;
```




+ For non-empty Traversables, see ` C\firstx `.
+ For possibly null Traversables, see ` C\nfirst `.
+ For single-element Traversables, see ` C\onlyx `.
+ For Awaitables that yield Traversables, see ` C\first_async `.




Time complexity: O(1)
Space complexity: O(1)




## Parameters




* [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




- ` ?T `




## Examples




``` basie-usage.hack
$strings = vec["a", "b", "c"];
$first_string = C\first($strings);
echo "First string in traversable: $first_string \n";
//Output: First string in traversable: a 

$empty_traversable = vec[];
$first_element = C\first($empty_traversable);
$first_element_as_string = $first_element ?? "null";
echo "First element in empty traversable: $first_element_as_string";
//Output: First element in empty traversable: null
```
<!-- HHAPIDOC -->
