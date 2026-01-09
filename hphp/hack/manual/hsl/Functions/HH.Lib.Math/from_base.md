
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Converts the given string in the given base to an int, assuming letters a-z
are used for digits when ` $from_base ` > 10




``` Hack
namespace HH\Lib\Math;

function from_base(
  string $number,
  int $from_base,
): int;
```




To base convert an int into a string, see ` Math\to_base() `.




## Parameters




+ ` string $number `
+ ` int $from_base `




## Returns




* ` int `




## Examples




``` basic-usage.hack
$val1 = Math\from_base("101", 2);
echo "101 in base 2 represents the number $val1 \n";

$val2 = Math\from_base("e", 16);
echo "e in base 16 represents the number $val2 \n";
```
<!-- HHAPIDOC -->
