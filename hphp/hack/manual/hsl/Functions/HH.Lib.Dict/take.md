
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict containing the first ` $n ` entries of the given
KeyedTraversable




``` Hack
namespace HH\Lib\Dict;

function take<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  int $n,
): dict<Tk, Tv>;
```




To drop the first ` $n ` entries, see `` Dict\drop() ``.




Time complexity: O(n), where n is ` $n `
Space complexity: O(n), where n is `` $n ``




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` int $n `




## Returns




* ` dict<Tk, Tv> `




## Examples




``` basic-usage.hack
$result = Dict\take(dict[1 => 2, 2 => 4, 3 => 9], 1);
print_r($result);
//dict[1=>2]

$result = Dict\take(dict[], 1);
print_r($result);
//result: dict[]
```
<!-- HHAPIDOC -->
