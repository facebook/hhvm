
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the last element from a Container and returns it




``` Hack
namespace HH\Lib\C;

function pop_backx<T as Container<Tv>, Tv>(
  inout T $container,
): Tv;
```




If the Container is empty, an [` InvariantException `](/apis/Classes/HH/InvariantException/) is thrown.




When an immutable Hack Collection is passed, the result will
be defined by your version of hhvm and not give the expected results.




For maybe empty Containers, see ` pop_back `.
To get the first element, see `` pop_frontx ``.




Time complexity: O(1 or N) If the operation can happen in-place, O(1)
if it must copy the Container, O(N).
Space complexity: O(1 or N) If the operation can happen in-place, O(1)
if it must copy the Container, O(N).




## Parameters




+ ` inout T $container `




## Returns




* ` Tv `




## Examples




``` basic-usage.hack
$strings = vec["a", "b"];
$pop_backx_result_1 = C\pop_backx(inout $strings);
echo "First pop_backx result: $pop_backx_result_1\n";
//Output: First pop_backx result: b

$empty_strings = vec[];
$pop_backx_result_2 = C\pop_backx(inout $empty_strings);
//Output: Hit a php exception : exception 'InvariantViolationException' with message 
//'HH\Lib\C\pop_backx: Expected at least one element'
```
<!-- HHAPIDOC -->
