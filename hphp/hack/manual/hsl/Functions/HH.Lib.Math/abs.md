
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the absolute value of ` $number ` (`` $number `` if ``` $number ``` > 0,
```` -$number ```` if ````` $number ````` < 0)




``` Hack
namespace HH\Lib\Math;

function abs<T as num>(
  T $number,
): T;
```




NB: for the smallest representable int, PHP_INT_MIN, the result is
"implementation-defined" because the corresponding positive number overflows
int. You will probably find that ` Math\abs(PHP_INT_MIN) === PHP_INT_MIN `,
meaning the function can return a negative result in that case. To ensure
an int is non-negative for hashing use `` $v & PHP_INT_MAX `` instead.




## Parameters




+ ` T $number `




## Returns




* ` T `




## Examples




``` basic-usage.hack
$negative_number = -25;
$negative_number_abs = Math\abs($negative_number);
echo "Negative test - before: $negative_number after: $negative_number_abs \n";

$positive_number = 25;
$positive_number_abs = Math\abs($positive_number);
echo "Positive test - before: $positive_number after: $positive_number_abs \n";
```
<!-- HHAPIDOC -->
