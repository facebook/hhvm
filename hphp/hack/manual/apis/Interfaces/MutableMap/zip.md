
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableMap `](/docs/apis/Interfaces/MutableMap/) where each value is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the
value of the current [` MutableMap `](/docs/apis/Interfaces/MutableMap/) and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): MutableMap<Tk, Pair<Tv, Tu>>;
```




If the number of values of the current [` MutableMap `](/docs/apis/Interfaces/MutableMap/) are not equal to the
number of elements in the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), then only the combined elements
up to and including the final element of the one with the least number of
elements is included.




The keys associated with the current [` MutableMap `](/docs/apis/Interfaces/MutableMap/) remain unchanged in the
returned [` MutableMap `](/docs/apis/Interfaces/MutableMap/).




## Parameters




+ [` Traversable<Tu> `](/docs/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of the current [` MutableMap `](/docs/apis/Interfaces/MutableMap/).




## Returns




* [` MutableMap<Tk, `](/docs/apis/Interfaces/MutableMap/)`` Pair<Tv, Tu>> `` - The [` MutableMap `](/docs/apis/Interfaces/MutableMap/) that combines the values of the current
  [` MutableMap `](/docs/apis/Interfaces/MutableMap/) with the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/).
<!-- HHAPIDOC -->
