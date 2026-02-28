
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Given a KeyedTraversable with nullable values, returns a new dict with
null values removed




``` Hack
namespace HH\Lib\Dict;

function filter_nulls<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, ?Tv> $traversable,
): dict<Tk, Tv>;
```




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` ?Tv> $traversable ``




## Returns




* ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
