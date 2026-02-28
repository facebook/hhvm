
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing the keys of the given KeyedTraversable,
maintaining the iteration order




``` Hack
namespace HH\Lib\Keyset;

function keys<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
): keyset<Tk>;
```




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``




## Returns




* ` keyset<Tk> `




## Examples




``` basic-usage.hack
$result = Keyset\keys(dict[1 => 100, 33 => 400 ]);
print_r($result);
//result: keyset[1,33]

$result = Keyset\keys(dict[]);
print_r($result);
//result: keyset[]
```
<!-- HHAPIDOC -->
