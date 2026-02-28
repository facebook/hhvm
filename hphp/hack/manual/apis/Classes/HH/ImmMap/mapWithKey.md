
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/apis/Classes/HH/ImmMap/) after an operation has been applied to each key and
value in current [` ImmMap `](/apis/Classes/HH/ImmMap/)




``` Hack
public function mapWithKey<Tu>(
  (function(Tk, Tv): Tu) $callback,
): ImmMap<Tk, Tu>;
```




Every key and value in the current [` ImmMap `](/apis/Classes/HH/ImmMap/) is affected by a call to
[` mapWithKey() `](/apis/Classes/HH/ImmMap/mapWithKey/), unlike [` filterWithKey() `](/apis/Classes/HH/ImmMap/filterWithKey/) where only values that meet a
certain criteria are affected.




The keys will remain unchanged from the current [` ImmMap `](/apis/Classes/HH/ImmMap/) to the returned
[` ImmMap `](/apis/Classes/HH/ImmMap/). The keys are only used to help in the operation.




## Parameters




+ ` (function(Tk, Tv): Tu) $callback `




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tu> `` - an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the values after a user-specified
  operation on the current [` ImmMap `](/apis/Classes/HH/ImmMap/)'s keys and values is applied.




## Examples




See [` Map::mapWithKey `](/apis/Classes/HH/Map/mapWithKey/#examples) for usage examples.
<!-- HHAPIDOC -->
