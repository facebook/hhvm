
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstMap `](/apis/Interfaces/ConstMap/) containing the values after the `` n ``-th element of the
current [` ConstMap `](/apis/Interfaces/ConstMap/)




``` Hack
public function skip(
  int $n,
): ConstMap<Tk, Tv>;
```




The returned [` ConstMap `](/apis/Interfaces/ConstMap/) will always be a proper subset of the current
[` ConstMap `](/apis/Interfaces/ConstMap/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` ConstMap `](/apis/Interfaces/ConstMap/).




## Returns




* [` ConstMap<Tk, `](/apis/Interfaces/ConstMap/)`` Tv> `` - A [` ConstMap `](/apis/Interfaces/ConstMap/) that is a proper subset of the current [` ConstMap `](/apis/Interfaces/ConstMap/)
  containing values after the specified `` n ``-th element.
<!-- HHAPIDOC -->
