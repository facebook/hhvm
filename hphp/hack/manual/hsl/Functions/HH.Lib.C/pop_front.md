
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Like ` pop_back `, but removes the first item




``` Hack
namespace HH\Lib\C;

function pop_front<T as Container<Tv>, Tv>(
  inout T $container,
): ?Tv;
```




Removes the first element from a Container and returns it.
If the Container is empty, null is returned.




When an immutable Hack Collection is passed, the result will
be defined by your version of hhvm and not give the expected results.




To enforce that the container is not empty, see ` pop_frontx `.
To get the last element, see `` pop_back ``.




Note that removing an item from the input array may not be "cheap." Keyed
containers such as ` dict ` can easily have the first item removed, but indexed
containers such as `` vec `` need to be wholly rewritten so the new [0] is the
old [1].




Time complexity: O(1 or N): If the operation can happen in-place, O(1);
if it must copy the Container, O(N).
Space complexity: O(1 or N): If the operation can happen in-place, O(1);
if it must copy the Container, O(N).




## Parameters




+ ` inout T $container `




## Returns




* ` ?Tv `




## Examples




``` basic-usage.hack
$strings = vec["a", "b"];
$pop_front_result_1 = C\pop_front(inout $strings);
echo "First pop_front result: $pop_front_result_1\n";
//Output: First pop_front result: a

$empty_strings = vec[];
$pop_front_result_2 = C\pop_front(inout $empty_strings);
$pop_front_result_2_as_string = $pop_front_result_2 ?? "null";
echo "Second pop_front result: $pop_front_result_2_as_string\n";
//Output: Second pop_front result: null
```
<!-- HHAPIDOC -->
