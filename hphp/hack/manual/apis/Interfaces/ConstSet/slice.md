
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` ConstSet `](/apis/Interfaces/ConstSet/) starting from a given key up
to, but not including, the element at the provided length from the
starting key




``` Hack
public function slice(
  int $start,
  int $len,
): ConstSet<Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Interfaces/ConstSet/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` ConstSet `](/apis/Interfaces/ConstSet/) will always be a proper subset of the current
[` ConstSet `](/apis/Interfaces/ConstSet/).




## Parameters




+ ` int $start ` - The starting value in the current [` ConstSet `](/apis/Interfaces/ConstSet/) for the
  returned [` ConstSet `](/apis/Interfaces/ConstSet/).
+ ` int $len ` - The length of the returned [` ConstSet `](/apis/Interfaces/ConstSet/).




## Returns




* [` ConstSet<Tv> `](/apis/Interfaces/ConstSet/) - A [` ConstSet `](/apis/Interfaces/ConstSet/) that is a proper subset of the current [` ConstSet `](/apis/Interfaces/ConstSet/)
  starting at `` $start `` up to but not including the element
  ``` $start + $len ```.
<!-- HHAPIDOC -->
