
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the given number rounded to the specified precision




``` Hack
namespace HH\Lib\Math;

function round(
  num $val,
  int $precision = 0,
): float;
```




A positive
precision rounds to the nearest decimal place whereas a negative precision
rounds to the nearest power of ten. For example, a precision of 1 rounds to
the nearest tenth whereas a precision of -1 rounds to the nearest ten.




## Parameters




+ ` num $val `
+ ` int $precision = 0 `




## Returns




* ` float `




## Examples




``` basic-usage.hack
$round1 = Math\round(8.65);
echo "8.65 rounded to default precision 0 yields $round1 \n";

$round2 = Math\round(8.65, 1);
echo "8.65 rounded to precision 1 yields $round2 \n";

$round3 = Math\round(8.65, -1);
echo "8.65 rounded to precision -1 yields $round3 \n";
```
<!-- HHAPIDOC -->
