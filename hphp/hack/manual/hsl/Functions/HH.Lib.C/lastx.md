
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the last element of the given Traversable, or throws if the
Traversable is empty




``` Hack
namespace HH\Lib\C;

function lastx<T>(
  Traversable<T> $traversable,
): T;
```




+ For possibly empty Traversables, see ` C\last `.
+ For single-element Traversables, see ` C\onlyx `.




Time complexity: O(1) if ` $traversable ` is a [` Container `](/apis/Interfaces/HH/Container/), O(n) otherwise.
Space complexity: O(1)




## Parameters




* [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




- ` T `




## Examples




``` basic-usage.hack
$strings = vec["a", "b", "c"];
$last_string = C\lastx($strings);
echo "Last string in traversable: $last_string \n";
//Output: Last string in traversable: c 

$empty_traversable = vec[];
$last_element = C\lastx($empty_traversable);
//Output: Hit a php exception : exception 'InvariantViolationException' with message
//'HH\Lib\C\lastx: Expected at least one element.'
```
<!-- HHAPIDOC -->
