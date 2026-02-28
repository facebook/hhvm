
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmMap `](/apis/Classes/HH/ImmMap/) containing the values after the `` n ``-th element of the
current [` ImmMap `](/apis/Classes/HH/ImmMap/)




``` Hack
public function skip(
  int $n,
): ImmMap<Tk, Tv>;
```




The returned [` ImmMap `](/apis/Classes/HH/ImmMap/) will always be a proper subset of the current
[` ImmMap `](/apis/Classes/HH/ImmMap/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` ImmMap `](/apis/Classes/HH/ImmMap/).




## Returns




* [` ImmMap<Tk, `](/apis/Classes/HH/ImmMap/)`` Tv> `` - An [` ImmMap `](/apis/Classes/HH/ImmMap/) that is a proper subset of the current [` ImmMap `](/apis/Classes/HH/ImmMap/)
  containing values after the specified `` n ``-th element.




## Examples




See [` Map::skip `](/apis/Classes/HH/Map/skip/#examples) for usage examples.
<!-- HHAPIDOC -->
