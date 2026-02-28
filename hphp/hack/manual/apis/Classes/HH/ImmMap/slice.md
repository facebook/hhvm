
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` ImmMap `](/apis/Classes/HH/ImmMap/) starting from a given key
location up to, but not including, the element at the provided length from
the starting key location




``` Hack
public function slice(
  int $start,
  int $len,
): ImmMap<Tk, Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Classes/HH/ImmMap/slice/)`` 2) `` would return the
keys and values at key location 0 and 1.




The returned [` ImmMap `](/apis/Classes/HH/ImmMap/) will always be a proper subset of the current
[` ImmMap `](/apis/Classes/HH/ImmMap/).




## Parameters




+ ` int $start ` - The starting key location of the current [` ImmMap `](/apis/Classes/HH/ImmMap/) for the
  returned [` ImmMap `](/apis/Classes/HH/ImmMap/).
+ ` int $len ` - The length of the returned [` ImmMap `](/apis/Classes/HH/ImmMap/).




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/apis/Classes/HH/ImmMap/) that is a proper subset of the current [` ImmMap `](/apis/Classes/HH/ImmMap/)
  starting at `` $start `` up to but not including the element
  ``` $start + $len ```.




## Examples




See [` Map::slice `](/apis/Classes/HH/Map/slice/#examples) for usage examples.
<!-- HHAPIDOC -->
