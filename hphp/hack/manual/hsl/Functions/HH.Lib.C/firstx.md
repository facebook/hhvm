
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first element of the given Traversable, or throws if the
Traversable is empty




``` Hack
namespace HH\Lib\C;

function firstx<T>(
  Traversable<T> $traversable,
): T;
```




+ For possibly empty Traversables, see ` C\first `.
+ For possibly null Traversables, see ` C\nfirst `.
+ For single-element Traversables, see ` C\onlyx `.
+ For Awaitables that yield Traversables, see ` C\firstx_async `.




Time complexity: O(1)
Space complexity: O(1)




## Parameters




* [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




- ` T `




## Examples




``` basie-usage.hack
$strings = vec["a", "b", "c"];
$first_string = C\firstx($strings);
echo "First string in traversable: $first_string \n";
//Output: First string in traversable: a 

$empty_traversable = vec[];
$first_element = C\firstx($empty_traversable);
//Output: Hit a php exception : exception 'InvariantViolationException' with message
//'HH\Lib\C\firstx: Expected at least one element.' 
```
<!-- HHAPIDOC -->
