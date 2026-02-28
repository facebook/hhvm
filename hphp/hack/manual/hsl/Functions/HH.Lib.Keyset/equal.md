
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the two given keysets have the same elements, using strict
equality




``` Hack
namespace HH\Lib\Keyset;

function equal<Tv as arraykey>(
  keyset<Tv> $keyset1,
  keyset<Tv> $keyset2,
): bool;
```




To guarantee equality of order as well as contents, use ` === `.




Time complexity: O(n)
Space complexity: O(1)




## Parameters




+ ` keyset<Tv> $keyset1 `
+ ` keyset<Tv> $keyset2 `




## Returns




* ` bool `




## Examples




``` basic-usage.hack
$result = Keyset\equal(keyset[1,2,3,4], keyset[1,2,3,4]);
print_r($result);
//result: true

$result = Keyset\equal(keyset[1,2,3,4], keyset[4,2,3,1]);
print_r($result);
//result: true

$result = Keyset\equal(keyset[1,2,3,4],  keyset[1,2,3,4,5]);
print_r($result);
//result: false
```
<!-- HHAPIDOC -->
