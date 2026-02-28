
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a 2-tuple containing keysets for which the given predicate returned
` true ` and `` false ``, respectively




``` Hack
namespace HH\Lib\Keyset;

function partition<Tv as arraykey>(
  Traversable<Tv> $traversable,
  (function(Tv): bool) $predicate,
): (keyset<Tv>, keyset<Tv>);
```




Time complexity: O(n * p), where p is the complexity of ` $predicate `
Space complexity: O(n)




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv): bool) $predicate `




## Returns




* ` (keyset<Tv>, keyset<Tv>) `




## Examples




``` basic-usage.hack
$result = Keyset\partition(keyset[1,2,3,4], $val ==> $val%2==0);
print_r($result);
//result: vec[keyset[2,4], keyset[1,3]]

$result = Keyset\partition(keyset[1,2,3,4], $val ==> $val==0);
print_r($result);
//result: vec[keyset[], keyset[1,2,3,4]]
```
<!-- HHAPIDOC -->
