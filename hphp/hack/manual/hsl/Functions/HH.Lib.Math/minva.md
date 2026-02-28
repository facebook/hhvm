
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the smallest of all input numbers




``` Hack
namespace HH\Lib\Math;

function minva<T as num>(
  T $first,
  T $second,
  T ...$rest,
): T;
```




+ To find the largest number, see ` Math\maxva() `.
+ For Traversables, see ` Math\min() `.




## Parameters




* ` T $first `
* ` T $second `
* ` T ...$rest `




## Returns




- ` T `




## Examples




``` basic-usage.hack
$minval = Math\minva(1, 2, 3, 4, 5, 5, 4);
echo "The min value is " . $minval;
```
<!-- HHAPIDOC -->
