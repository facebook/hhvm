
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Just like filter, but your predicate can include the key as well as
the value




``` Hack
namespace HH\Lib\Dict;

function filter_with_key<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $traversable,
  (function(Tk, Tv): bool) $predicate,
): dict<Tk, Tv>;
```




To use an async predicate, see ` Dict\filter_with_key_async() `.




Time complexity: O(n * p), where p is the complexity of ` $value_predicate `
(which is O(1) if not provided explicitly)
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable ``
+ ` (function(Tk, Tv): bool) $predicate `




## Returns




* ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
