
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an [` ImmMap `](/apis/Classes/HH/ImmMap/) from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty [` ImmMap `](/apis/Classes/HH/ImmMap/)
if `` null `` is passed




``` Hack
public static function fromItems(
  ?Traversable<Pair<Tk, Tv>> $iterable,
): ImmMap<Tk, Tv>;
```




This is the static method version of the [` ImmMap::__construct() `](/apis/Classes/HH/ImmMap/__construct/)
constructor.




## Parameters




+ ` ? `[` Traversable<Pair<Tk, `](/apis/Interfaces/HH/Traversable/)`` Tv>> $iterable ``




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/apis/Classes/HH/ImmMap/) with the key/value pairs from the [` Traversable `](/apis/Interfaces/HH/Traversable/); or
  an empty [` ImmMap `](/apis/Classes/HH/ImmMap/) if the [` Traversable `](/apis/Interfaces/HH/Traversable/) is `` null ``.




## Examples




See [` Map::fromItems `](/apis/Classes/HH/Map/fromItems/#examples) for usage examples.
<!-- HHAPIDOC -->
