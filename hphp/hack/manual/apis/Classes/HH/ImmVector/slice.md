
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` ImmVector `](/apis/Classes/HH/ImmVector/) starting from a given key up
to, but not including, the element at the provided length from the
starting key




``` Hack
public function slice(
  int $start,
  int $len,
): ImmVector<Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Classes/HH/ImmVector/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` ImmVector `](/apis/Classes/HH/ImmVector/) will always be a subset (but not necessarily a
proper subset) of the current [` ImmVector `](/apis/Classes/HH/ImmVector/). If `` $start `` is greater than or
equal to the length of the current [` Vector `](/apis/Classes/HH/Vector/), the returned [` Vector `](/apis/Classes/HH/Vector/) will
contain no elements.  If `` $start `` + ``` $len ``` is greater than or equal to the
length of the current [` Vector `](/apis/Classes/HH/Vector/), the returned [` Vector `](/apis/Classes/HH/Vector/) will contain the
elements from `` $start `` to the end of the current [` Vector `](/apis/Classes/HH/Vector/).




If either ` $start ` or `` $len `` is negative, an exception is thrown.




## Parameters




+ ` int $start ` - The starting key of the current [` ImmVector `](/apis/Classes/HH/ImmVector/) at which to
  begin the returned [` ImmVector `](/apis/Classes/HH/ImmVector/).
+ ` int $len ` - The length of the returned [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Returns




* [` ImmVector<Tv> `](/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/apis/Classes/HH/ImmVector/) that is a subset of the current [` ImmVector `](/apis/Classes/HH/ImmVector/)
  starting at `` $start `` up to but not including the element
  ``` $start + $len ```.




## Examples




See [` Vector::slice `](/apis/Classes/HH/Vector/slice/#examples) for usage examples.
<!-- HHAPIDOC -->
