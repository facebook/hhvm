
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/apis/Interfaces/MutableSet/) containing the values after the `` n ``-th element of
the current [` MutableSet `](/apis/Interfaces/MutableSet/)




``` Hack
public function skip(
  int $n,
): MutableSet<Tv>;
```




The returned [` MutableSet `](/apis/Interfaces/MutableSet/) will always be a proper subset of the current
[` MutableSet `](/apis/Interfaces/MutableSet/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element to be skipped; the `` $n+1 `` element will be the
  first one in the returned [` MutableSet `](/apis/Interfaces/MutableSet/).




## Returns




* [` MutableSet<Tv> `](/apis/Interfaces/MutableSet/) - A [` MutableSet `](/apis/Interfaces/MutableSet/) that is a proper subset of the current
  [` MutableSet `](/apis/Interfaces/MutableSet/) containing values after the specified `` n ``-th
  element.
<!-- HHAPIDOC -->
