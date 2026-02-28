
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset formed by joining the values
within the given Traversables into
a keyset




``` Hack
namespace HH\Lib\Keyset;

function flatten<Tv as arraykey>(
  Traversable<Container<Tv>> $traversables,
): keyset<Tv>;
```




For a fixed number of Traversables, see ` Keyset\union() `.




Time complexity: O(n), where n is the combined size of all the
` $traversables `
Space complexity: O(n), where n is the combined size of all the
`` $traversables ``




## Parameters




+ [` Traversable<Container<Tv>> `](/apis/Interfaces/HH/Traversable/)`` $traversables ``




## Returns




* ` keyset<Tv> `




## Examples




``` basic-usage.hack
$example_set = vec[keyset[1,2,3,4,5],keyset[98, 99]];
$result = Keyset\flatten($example_set);
print_r($result);
//result: keyset[1,2,3,4,5,98,99]

$example_set = vec[vec[1,2,3,4,5,5,5,5]];
$result = Keyset\flatten($example_set);
print_r($result);
//result: keyset[1,2,3,4,5]

$example_set = vec[];
$result = Keyset\flatten($example_set);
print_r($result);
//result: keyset[]
```
<!-- HHAPIDOC -->
