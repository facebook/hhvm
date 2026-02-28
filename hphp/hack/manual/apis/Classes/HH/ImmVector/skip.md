
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values after the `` $n ``-th element of
the current [` ImmVector `](/apis/Classes/HH/ImmVector/)




``` Hack
public function skip(
  int $n,
): ImmVector<Tv>;
```




The returned [` ImmVector `](/apis/Classes/HH/ImmVector/) will always be a subset (but not necessarily a
proper subset) of the current [` ImmVector `](/apis/Classes/HH/ImmVector/). If `` $n `` is greater than or equal
to the length of the current [` ImmVector `](/apis/Classes/HH/ImmVector/), the returned [` ImmVector `](/apis/Classes/HH/ImmVector/) will
contain no elements. If `` $n `` is negative, the returned [` ImmVector `](/apis/Classes/HH/ImmVector/) will
contain all elements of the current [` ImmVector `](/apis/Classes/HH/ImmVector/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Returns




* [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) that is a subset of the current [` ImmVector `](/apis/Classes/HH/ImmVector/)
  containing values after the specified `` $n ``-th element.




## Examples




See [` Vector::skip `](/apis/Classes/HH/Vector/skip/#examples) for usage examples.
<!-- HHAPIDOC -->
