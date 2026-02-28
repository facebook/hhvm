
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) containing the values after the `` n ``-th element of
the current [` ConstVector `](/apis/Interfaces/ConstVector/)




``` Hack
public function skip(
  int $n,
): ConstVector<Tv>;
```




The returned [` ConstVector `](/apis/Interfaces/ConstVector/) will always be a proper subset of the current
[` ConstVector `](/apis/Interfaces/ConstVector/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the $n+1 element will be the
  first one in the returned [` ConstVector `](/apis/Interfaces/ConstVector/).




## Returns




* [` ConstVector<Tv> `](/apis/Interfaces/ConstVector/) - A [` ConstVector `](/apis/Interfaces/ConstVector/) that is a proper subset of the current
  [` ConstVector `](/apis/Interfaces/ConstVector/) containing values after the specified `` n ``-th
  element.
<!-- HHAPIDOC -->
