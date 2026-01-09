
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the largest of all input numbers




``` Hack
namespace HH\Lib\Math;

function maxva<T as num>(
  T $first,
  T $second,
  T ...$rest,
): T;
```




+ To find the smallest number, see ` Math\minva() `.
+ For Traversables, see ` Math\max() `.




## Parameters




* ` T $first `
* ` T $second `
* ` T ...$rest `




## Returns




- ` T `




## Examples




``` basic-usage.hack
$maxval = Math\maxva(4, 3, 8, 1, 1, 2);
echo "The max value is " . $maxval;
```
<!-- HHAPIDOC -->
