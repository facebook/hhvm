
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the arithmetic mean of the numbers in the given container




``` Hack
namespace HH\Lib\Math;

function mean(
  Container<num> $numbers,
): ?float;
```




+ To find the sum, see ` Math\sum() `.
+ To find the maximum, see ` Math\max() `.
+ To find the minimum, see ` Math\min() `.




## Parameters




* [` Container<num> `](/apis/Interfaces/HH/Container/)`` $numbers ``




## Returns




- ` ?float `




## Examples




``` basic-usage.hack
$v = vec[1, 2, 3, 4, 5];
$mean = Math\mean($v);
echo "Mean of the array is $mean \n"; // Output: Mean of the array is 3 
```
<!-- HHAPIDOC -->
