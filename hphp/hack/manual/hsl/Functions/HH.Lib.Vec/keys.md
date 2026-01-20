
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec containing the keys of the given KeyedTraversable




``` Hack
namespace HH\Lib\Vec;

function keys<Tk, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
): vec<Tk>;
```




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``




## Returns




* ` vec<Tk> `




## Examples




``` basic-usage.hack
$result = Vec\keys(dict[1 => 100, 33 => 400 ]);
print_r($result);
//result: [1, 33]

$result = Vec\keys(dict[]);
print_r($result);
//result: []
```
<!-- HHAPIDOC -->
