
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Converts the given string in base ` $from_base ` to base `` $to_base ``, assuming
letters a-z are used for digits for bases greater than 10




``` Hack
namespace HH\Lib\Math;

function base_convert(
  string $value,
  int $from_base,
  int $to_base,
): string;
```




The conversion is
done to arbitrary precision.




+ To convert a string in some base to an int, see ` Math\from_base() `.
+ To convert an int to a string in some base, see ` Math\to_base() `.




## Parameters




* ` string $value `
* ` int $from_base `
* ` int $to_base `




## Returns




- ` string `




## Examples




``` basic-usage.hack
$result1 = Math\base_convert("30", 10, 16);
echo "30 in base 10 is $result1 in base 16 \n";

$result2 = Math\base_convert("2f", 16, 10);
echo "2f in base 16 is $result2 in base 10 \n";

$result3 = Math\base_convert("1111", 2, 16);
echo "1111 in base 2 is $result3 in base 16 \n";
```
<!-- HHAPIDOC -->
