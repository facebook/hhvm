
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the result of integer division of ` $numerator ` by `` $denominator ``




``` Hack
namespace HH\Lib\Math;

function int_div(
  int $numerator,
  int $denominator,
): int;
```




To round a single value, see ` Math\floor() `.




## Parameters




+ ` int $numerator `
+ ` int $denominator `




## Returns




* ` int `




## Examples




``` basic-usage.hack
$result1 = Math\int_div(8, 4);
echo "Integer division of 8 by 4 yields $result1 \n";

$result2 = Math\int_div(7, 4);
echo "Integer division of 7 by 4 yields $result2 \n";

```
<!-- HHAPIDOC -->
