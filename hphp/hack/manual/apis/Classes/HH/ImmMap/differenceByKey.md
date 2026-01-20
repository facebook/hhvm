
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new [` ImmMap `](/apis/Classes/HH/ImmMap/) with the keys that are in the current [` ImmMap `](/apis/Classes/HH/ImmMap/), but
not in the provided [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/)




``` Hack
public function differenceByKey(
  KeyedTraversable<mixed, mixed> $traversable,
): ImmMap<Tk, Tv>;
```




## Parameters




+ [` KeyedTraversable<mixed, `](/apis/Interfaces/HH/KeyedTraversable/)`` mixed> $traversable `` - The [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/) on which to compare the keys.




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the keys (and associated values) of the
  current [` ImmMap `](/apis/Classes/HH/ImmMap/) that are not in the [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/).




## Examples




See [`Map::differenceByKey`](/apis/Classes/HH/Map/differenceByKey/#examples) for usage examples.
<!-- HHAPIDOC -->
