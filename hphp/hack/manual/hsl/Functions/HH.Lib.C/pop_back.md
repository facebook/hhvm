
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the last element from a Container and returns it




``` Hack
namespace HH\Lib\C;

function pop_back<T as Container<Tv>, Tv>(
  inout T $container,
): ?Tv;
```




If the Container is empty, null will be returned.




When an immutable Hack Collection is passed, the result will
be defined by your version of hhvm and not give the expected results.




For non-empty Containers, see ` pop_backx `.
To get the first element, see `` pop_front ``.




Time complexity: O(1 or N) If the operation can happen in-place, O(1)
if it must copy the Container, O(N).
Space complexity: O(1 or N) If the operation can happen in-place, O(1)
if it must copy the Container, O(N).




## Parameters




+ ` inout T $container `




## Returns




* ` ?Tv `




## Examples




``` basic-usage.hack
$strings = vec["a", "b"];
$pop_back_result_1 = C\pop_back(inout $strings);
echo "First pop_back result: $pop_back_result_1\n";
//Output: First pop_back result: b

$empty_strings = vec[];
$pop_back_result_2 = C\pop_back(inout $empty_strings);
$pop_back_result_2_as_string = $pop_back_result_2 ?? "null";
echo "Second pop_back result: $pop_back_result_2_as_string\n";
//Output: Second pop_back result: null
```
<!-- HHAPIDOC -->
