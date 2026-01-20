
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the integer sum of the values of the given Traversable




``` Hack
namespace HH\Lib\Math;

function sum(
  Traversable<int> $traversable,
): int;
```




For a float sum, see ` Math\sum_float() `.




## Parameters




+ [` Traversable<int> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




* ` int `




## Examples




``` basic-usage.hack
$v = vec[2, 1, 4];
$sum = Math\sum($v);
echo "Sum of array is $sum \n";
```
<!-- HHAPIDOC -->
