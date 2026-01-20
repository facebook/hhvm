
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` ConstVector `](/apis/Interfaces/ConstVector/) starting from a given key up
to, but not including, the element at the provided length from the starting
key




``` Hack
public function slice(
  int $start,
  int $len,
): ConstVector<Tv>;
```




` $start ` is 0-based. $len is 1-based. So [` slice(0, `](/apis/Interfaces/ConstVector/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` ConstVector `](/apis/Interfaces/ConstVector/) will always be a proper subset of this
[` ConstVector `](/apis/Interfaces/ConstVector/).




## Parameters




+ ` int $start ` - The starting key of this Vector to begin the returned
  [` ConstVector `](/apis/Interfaces/ConstVector/).
+ ` int $len ` - The length of the returned [` ConstVector `](/apis/Interfaces/ConstVector/).




## Returns




* [` ConstVector<Tv> `](/apis/Interfaces/ConstVector/) - A [` ConstVector `](/apis/Interfaces/ConstVector/) that is a proper subset of the current
  [` ConstVector `](/apis/Interfaces/ConstVector/) starting at `` $start `` up to but not including the
  element ``` $start + $len ```.
<!-- HHAPIDOC -->
