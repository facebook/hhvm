
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict keyed by the values of the given KeyedTraversable
and vice-versa




``` Hack
namespace HH\Lib\Dict;

function flip<Tk, Tv as arraykey>(
  KeyedTraversable<Tk, Tv> $traversable,
): dict<Tv, Tk>;
```




In case of duplicate values, later keys overwrite the
previous ones.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``




## Returns




* ` dict<Tv, Tk> `
<!-- HHAPIDOC -->
