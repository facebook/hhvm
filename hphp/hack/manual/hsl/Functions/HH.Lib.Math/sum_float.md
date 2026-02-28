
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the float sum of the values of the given Traversable




``` Hack
namespace HH\Lib\Math;

function sum_float<T as num>(
  Traversable<T> $traversable,
): float;
```




For an integer sum, see ` Math\sum() `.




## Parameters




+ [` Traversable<T> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




* ` float `




## Examples




``` basic-usage.hack
$v = vec[2.3, 1, 4.15];
$sum = Math\sum_float($v);
echo "Float sum of array is $sum \n";
```
<!-- HHAPIDOC -->
