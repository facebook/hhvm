
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) where each value is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the value
of the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): ImmMap<Tk, Pair<Tv, Tu>>;
```




If the number of values of the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) are not equal to the
number of elements in the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), then only the combined elements
up to and including the final element of the one with the least number of
elements is included.




The keys associated with the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) remain unchanged in the
returned [` ImmMap `](/docs/apis/Classes/HH/ImmMap/).




## Parameters




+ [` Traversable<Tu> `](/docs/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/).




## Returns




* [` ImmMap<Tk, `](/docs/apis/Classes/HH/ImmMap/)`` Pair<Tv, Tu>> `` - The [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) that combines the values of the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/)
  with the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/).




## Examples




See [` Map::zip `](/docs/apis/Classes/HH/Map/zip/#examples) for usage examples.
<!-- HHAPIDOC -->
