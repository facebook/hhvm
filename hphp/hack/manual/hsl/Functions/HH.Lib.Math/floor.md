
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the largest integer value less than or equal to ` $value `




``` Hack
namespace HH\Lib\Math;

function floor(
  num $value,
): float;
```




+ To find the smallest integer value greater than or equal to ` $value `, see
  `` Math\ceil() ``.
+ To find the largest integer value less than or equal to a ratio, see
  ` Math\int_div() `.




## Parameters




* ` num $value `




## Returns




- ` float `




## Examples




``` basic-usage.hack
$floor1 = Math\floor(4.7);
echo "Floor of 4.7 is $floor1 \n";

$floor2 = Math\floor(5);
echo "Floor or 5 is $floor2 \n";
```
<!-- HHAPIDOC -->
