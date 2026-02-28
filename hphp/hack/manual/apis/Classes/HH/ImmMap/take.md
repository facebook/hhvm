
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the first `` n `` key/values of the current
[` ImmMap `](/apis/Classes/HH/ImmMap/)




``` Hack
public function take(
  int $n,
): ImmMap<Tk, Tv>;
```




The returned [` ImmMap `](/apis/Classes/HH/ImmMap/) will always be a proper subset of the current
[` ImmMap `](/apis/Classes/HH/ImmMap/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the returned
  [` ImmMap `](/apis/Classes/HH/ImmMap/).




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/apis/Classes/HH/ImmMap/) that is a proper subset of the current [` ImmMap `](/apis/Classes/HH/ImmMap/) up
  to `` n `` elements.




## Examples




See [` Map::take `](/apis/Classes/HH/Map/take/#examples) for usage examples.
<!-- HHAPIDOC -->
