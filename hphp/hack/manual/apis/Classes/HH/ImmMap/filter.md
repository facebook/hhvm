
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the values of the current [` ImmMap `](/apis/Classes/HH/ImmMap/) that
meet a supplied condition




``` Hack
public function filter(
  (function(Tv): bool) $callback,
): ImmMap<Tk, Tv>;
```




Only values that meet a certain criteria are affected by a call to
[` filter() `](/apis/Classes/HH/ImmMap/filter/), while all values are affected by a call to [` map() `](/apis/Classes/HH/ImmMap/map/).




The keys associated with the current [` ImmMap `](/apis/Classes/HH/ImmMap/) remain unchanged in the
returned [` Map `](/apis/Classes/HH/Map/).




## Guide




+ [Examples](</hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): bool) $callback `




## Returns




- [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the values after a user-specified
  condition is applied.




## Examples




See [`Map::filter`](/apis/Classes/HH/Map/filter/#examples) for usage examples.
<!-- HHAPIDOC -->
