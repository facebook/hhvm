
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` MutableSet `](/apis/Interfaces/MutableSet/) starting from a given key up
to, but not including, the element at the provided length from the
starting key




``` Hack
public function slice(
  int $start,
  int $len,
): MutableSet<Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Interfaces/MutableSet/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` MutableSet `](/apis/Interfaces/MutableSet/) will always be a proper subset of the current
[` MutableSet `](/apis/Interfaces/MutableSet/).




## Parameters




+ ` int $start ` - The starting value in the current [` MutableSet `](/apis/Interfaces/MutableSet/) for the
  returned [` MutableSet `](/apis/Interfaces/MutableSet/).
+ ` int $len ` - The length of the returned [` MutableSet `](/apis/Interfaces/MutableSet/).




## Returns




* [` MutableSet<Tv> `](/apis/Interfaces/MutableSet/) - A [` MutableSet `](/apis/Interfaces/MutableSet/) that is a proper subset of the current
  [` MutableSet `](/apis/Interfaces/MutableSet/) starting at `` $start `` up to but not including the
  element ``` $start + $len ```.
<!-- HHAPIDOC -->
