
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the sine of $arg




``` Hack
namespace HH\Lib\Math;

function sin(
  num $arg,
): float;
```




+ To find the cosine, see ` Math\cos() `.
+ To find the tangent, see ` Math\tan() `.




## Parameters




* ` num $arg `




## Returns




- ` float `




## Examples




``` basic-usage.hack
$rotation = 0;
$result = Math\sin($rotation);
echo "Math\sin of $rotation : $result \n";

$rotation = 90;
$result = Math\sin($rotation);
echo "Math\sin of $rotation : $result \n";

$rotation = 180;
$result = Math\sin($rotation);
echo "Math\sin of $rotation : $result \n";
```
<!-- HHAPIDOC -->
