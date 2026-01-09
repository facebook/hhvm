
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Like ` pop_front ` but enforces non-empty container as input




``` Hack
namespace HH\Lib\C;

function pop_frontx<T as Container<Tv>, Tv>(
  inout T $container,
): Tv;
```




## Parameters




+ ` inout T $container `




## Returns




* ` Tv `




## Examples




``` basic-usage.hack
$strings = vec["a", "b"];
$pop_frontx_result_1 = C\pop_frontx(inout $strings);
echo "First pop_frontx result: $pop_frontx_result_1\n";
//Output: First pop_frontx result: a

$empty_strings = vec[];
$pop_frontx_result_2 = C\pop_frontx(inout $empty_strings);
//Output: Hit a php exception : exception 'InvariantViolationException' with message
//'HH\Lib\C\pop_frontx: Expected at least one element'
```
<!-- HHAPIDOC -->
