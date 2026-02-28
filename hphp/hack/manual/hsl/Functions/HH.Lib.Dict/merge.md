
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Merges multiple KeyedTraversables into a new dict




``` Hack
namespace HH\Lib\Dict;

function merge<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $first,
  KeyedContainer<Tk, Tv> ...$rest,
): dict<Tk, Tv>;
```




In the case of duplicate
keys, later values will overwrite the previous ones.




Time complexity: O(n + m), where n is the size of ` $first ` and m is the
combined size of all the `` ...$rest ``
Space complexity: O(n + m), where n is the size of ``` $first ``` and m is the
combined size of all the ```` ...$rest ````




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $first ``
+ [` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` Tv> ...$rest ``




## Returns




* ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
$result = Dict\merge(dict[1 => 2, 2 => 4], dict[7 => 2, 100 => 4]);
print_r($result);
//result: dict[1=>2, 2=>4, 7=>2, 100=>4]

$result = Dict\merge(dict[1 => 2, 2 => 4], dict[7 => 2, 2 => 100]);
print_r($result);
//result: dict[1=>2, 2=>100, 7=>2]
```
<!-- HHAPIDOC -->
