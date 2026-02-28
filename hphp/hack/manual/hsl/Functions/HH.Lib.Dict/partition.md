
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a 2-tuple containing dicts for which the given predicate returned
` true ` and `` false ``, respectively




``` Hack
namespace HH\Lib\Dict;

function partition<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tv): bool) $predicate,
): (dict<Tk, Tv>, dict<Tk, Tv>);
```




Time complexity: O(n * p), where p is the complexity of ` $predicate `.
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` (function(Tv): bool) $predicate `




## Returns




* ` (dict<Tk, Tv>, dict<Tk, Tv>) `
<!-- HHAPIDOC -->
