
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the two given dicts have the same entries, using strict
equality




``` Hack
namespace HH\Lib\Dict;

function equal<Tk as arraykey, Tv>(
  dict<Tk, Tv> $dict1,
  dict<Tk, Tv> $dict2,
): bool;
```




To guarantee equality of order as well as contents, use ` === `.




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ ` dict<Tk, Tv> $dict1 `
+ ` dict<Tk, Tv> $dict2 `




## Returns




* ` bool `




## Examples




``` basic-usage.hack
$result = Dict\equal(dict[], dict[]);
print_r($result);
//result: true

$result = Dict\equal(dict[1 => 2, 2 => 4], dict[1 => 2, 2 => 4]);
print_r($result);
//result: true

$result = Dict\equal(dict[1 => 2, 2 => 4], dict[1 => 2, 300 => 4]);
print($result);
//result: false

$result = Dict\equal(dict[1 => 2, 2 => 4], dict[1 => 2, 2 => 7000]);
//result: false
```
<!-- HHAPIDOC -->
