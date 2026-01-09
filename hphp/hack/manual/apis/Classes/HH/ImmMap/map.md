
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) after an operation has been applied to each value in
the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/)




``` Hack
public function map<Tu>(
  (function(Tv): Tu) $callback,
): ImmMap<Tk, Tu>;
```




Every value in the current [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) is affected by a call to [` map() `](/docs/apis/Classes/HH/ImmMap/map/),
unlike [` filter() `](/docs/apis/Classes/HH/ImmMap/filter/) where only values that meet a certain criteria are
affected.




The keys will remain unchanged from this [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) to the returned [` ImmMap `](/docs/apis/Classes/HH/ImmMap/).




## Guide




+ [Examples](</docs/hack/arrays-and-collections/introduction>)







## Parameters




* ` (function(Tv): Tu) $callback `




## Returns




- [` ImmMap<Tk, `](/docs/apis/Classes/HH/ImmMap/)`` Tu> `` - an [` ImmMap `](/docs/apis/Classes/HH/ImmMap/) containing key/value pairs after a user-specified
  operation is applied.




## Examples




See [` Map::map `](/docs/apis/Classes/HH/Map/map/#examples) for usage examples.
<!-- HHAPIDOC -->
