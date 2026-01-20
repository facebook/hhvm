
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` ImmSet `](/apis/Classes/HH/ImmSet/) starting from a given key up to,
but not including, the element at the provided length from the starting
key




``` Hack
public function slice(
  int $start,
  int $len,
): ImmSet<Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Classes/HH/ImmSet/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` ImmSet `](/apis/Classes/HH/ImmSet/) will always be a proper subset of the current
[` ImmSet `](/apis/Classes/HH/ImmSet/).




## Parameters




+ ` int $start ` - The starting value in the current [` ImmSet `](/apis/Classes/HH/ImmSet/) for the
  returned [` ImmSet `](/apis/Classes/HH/ImmSet/).
+ ` int $len ` - The length of the returned [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Returns




* [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/apis/Classes/HH/ImmSet/) that is a proper subset of the current [` ImmSet `](/apis/Classes/HH/ImmSet/)
  starting at `` $start `` up to but not including the element
  ``` $start + $len ```.




## Examples




See [` Set::slice `](/apis/Classes/HH/Set/slice/#examples) for usage examples.
<!-- HHAPIDOC -->
