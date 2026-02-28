
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Converts the given non-negative number into the given base, using letters a-z
for digits when ` $to_base ` > 10




``` Hack
namespace HH\Lib\Math;

function to_base(
  int $number,
  int $to_base,
): string;
```




To base convert a string to an int, see ` Math\from_base() `.




## Parameters




+ ` int $number `
+ ` int $to_base `




## Returns




* ` string `




## Examples




``` basic-usage.hack
$in = 30;

$base_1 = 10;
$out_1 = Math\to_base($in, $base_1);
echo "$in to base $base_1 is $out_1 \n";

$base_2 = 16;
$out_2 = Math\to_base($in, $base_2);
echo "$in to base $base_2 is $out_2 \n";

$base_3 = 2;
$out_3 = Math\to_base($in, $base_3);
echo "$in to base $base_3 is $out_3 \n";
```
<!-- HHAPIDOC -->
