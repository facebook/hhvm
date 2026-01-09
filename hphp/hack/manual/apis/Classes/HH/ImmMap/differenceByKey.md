
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) with the keys that are in the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/), but
not in the provided [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
public function differenceByKey(
  KeyedTraversable<mixed, mixed> $traversable,
): ImmMap<Tk, Tv>;
```




## Parameters




+ [` KeyedTraversable<mixed, `](/docs/apis/Interfaces/HH/KeyedTraversable/)`` mixed> $traversable `` - The [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/) on which to compare the keys.




## Returns




* [` ImmMap<Tk, `](/docs/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) containing the keys (and associated values) of the
  current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) that are not in the [` KeyedTraversable `](/docs/apis/Interfaces/HH/KeyedTraversable/).




## Examples




See [`Map::differenceByKey`](/docs/apis/Classes/HH/Map/differenceByKey/#examples) for usage examples.
<!-- HHAPIDOC -->
