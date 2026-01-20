
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstSet `](/apis/Interfaces/ConstSet/) containing the values after the `` n ``-th element of the
current [` ConstSet `](/apis/Interfaces/ConstSet/)




``` Hack
public function skip(
  int $n,
): ConstSet<Tv>;
```




The returned [` ConstSet `](/apis/Interfaces/ConstSet/) will always be a proper subset of the current
[` ConstSet `](/apis/Interfaces/ConstSet/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` ConstSet `](/apis/Interfaces/ConstSet/).




## Returns




* [` ConstSet<Tv> `](/apis/Interfaces/ConstSet/) - A [` ConstSet `](/apis/Interfaces/ConstSet/) that is a proper subset of the current [` ConstSet `](/apis/Interfaces/ConstSet/)
  containing values after the specified `` n ``-th element.
<!-- HHAPIDOC -->
