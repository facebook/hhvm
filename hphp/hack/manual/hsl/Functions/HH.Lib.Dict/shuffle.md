
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict with the key value pairs of the given input container in a random
order




``` Hack
namespace HH\Lib\Dict;

function shuffle<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Tv> $container,
): dict<Tk, Tv>;
```




Dict\\shuffle is not using cryptographically secure randomness.




Time complexity: O(n)
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $container ``




## Returns




* ` dict<Tk, Tv> `
<!-- HHAPIDOC -->
