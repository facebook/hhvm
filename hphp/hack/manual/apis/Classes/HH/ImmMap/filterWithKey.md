
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the values of the current [` ImmMap `](/apis/Classes/HH/ImmMap/) that
meet a supplied condition applied to its keys and values




``` Hack
public function filterWithKey(
  (function(Tk, Tv): bool) $callback,
): ImmMap<Tk, Tv>;
```




Only keys and values that meet a certain criteria are affected by a call to
[` filterWithKey() `](/apis/Classes/HH/ImmMap/filterWithKey/), while all values are affected by a call to
[` mapWithKey() `](/apis/Classes/HH/ImmMap/mapWithKey/).




The keys associated with the current [` ImmMap `](/apis/Classes/HH/ImmMap/) remain unchanged in the
returned [` ImmMap `](/apis/Classes/HH/ImmMap/); the keys will be used in the filtering process only.




## Parameters




+ ` (function(Tk, Tv): bool) $callback `




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the values after a user-specified
  condition is applied to the keys and values of the current
  [` ImmMap `](/apis/Classes/HH/ImmMap/).




## Examples




See [`Map::filterWithKey`](/apis/Classes/HH/Map/filterWithKey/#examples) for usage examples.
<!-- HHAPIDOC -->
