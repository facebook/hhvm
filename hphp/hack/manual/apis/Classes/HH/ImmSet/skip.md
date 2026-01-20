
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmSet `](/apis/Classes/HH/ImmSet/) containing the values after the `` n ``-th element of the
current [` ImmSet `](/apis/Classes/HH/ImmSet/)




``` Hack
public function skip(
  int $n,
): ImmSet<Tv>;
```




The returned [` ImmSet `](/apis/Classes/HH/ImmSet/) will always be a proper subset of the current
[` ImmSet `](/apis/Classes/HH/ImmSet/).




` n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Returns




* [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/apis/Classes/HH/ImmSet/) that is a proper subset of the current [` ImmSet `](/apis/Classes/HH/ImmSet/)
  containing values after the specified `` n ``-th element.




## Examples




See [` Set::skip `](/apis/Classes/HH/Set/skip/#examples) for usage examples.
<!-- HHAPIDOC -->
