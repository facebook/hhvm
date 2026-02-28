
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict in which each value appears exactly once




``` Hack
namespace HH\Lib\Dict;

function unique<Tk as arraykey, Tv as arraykey>(
  KeyedTraversable<Tk, Tv> $traversable,
): dict<Tk, Tv>;
```




In case of
duplicate values, later keys will overwrite the previous ones.




For non-arraykey values, see ` Dict\unique_by() `.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``




## Returns




* ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
