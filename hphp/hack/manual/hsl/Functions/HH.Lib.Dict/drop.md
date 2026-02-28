
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict containing all except the first ` $n ` entries of the
given KeyedTraversable




``` Hack
namespace HH\Lib\Dict;

function drop<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  int $n,
): dict<Tk, Tv>;
```




To take only the first ` $n ` entries, see `` Dict\take() ``.




Time complexity: O(n), where n is the size of ` $traversable `
Space complexity: O(n), where n is the size of `` $traversable ``




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` int $n `




## Returns




* ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
$result = Dict\drop(dict[1 => 2, 2 => 4, 3 => 9], 1);
print_r($result);
//result: dict[2=>4, 3=>9]

$result = Dict\drop(dict[], 1);
print_r($result);
//result: dict[]
```
<!-- HHAPIDOC -->
