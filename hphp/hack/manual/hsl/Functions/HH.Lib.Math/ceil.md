
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the smallest integer value greater than or equal to $value




``` Hack
namespace HH\Lib\Math;

function ceil(
  num $value,
): float;
```




To find the largest integer value less than or equal to ` $value `, see
`` Math\floor() ``.




## Parameters




+ ` num $value `




## Returns




* ` float `




## Examples




``` basic-usage.hack
$val_1 = 3.2;
$ceil_1 = Math\ceil($val_1);
echo "Ceiling of $val_1 is $ceil_1 \n";

$val_2 = 7;
$ceil_2 = Math\ceil($val_2);
echo "Ceiling of $val_2 is $ceil_2 \n";
```
<!-- HHAPIDOC -->
