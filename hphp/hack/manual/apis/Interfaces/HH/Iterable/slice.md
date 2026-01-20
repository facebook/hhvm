
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` Iterable `](/apis/Interfaces/HH/Iterable/) starting from a given key up
to, but not including, the element at the provided length from the
starting key




``` Hack
public function slice(
  int $start,
  int $len,
): Iterable<Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Interfaces/HH/Iterable/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` Iterable `](/apis/Interfaces/HH/Iterable/) will always be a proper subset of the current
[` Iterable `](/apis/Interfaces/HH/Iterable/).




## Parameters




+ ` int $start ` - The starting key of the current [` Iterable `](/apis/Interfaces/HH/Iterable/) to begin the
  returned [` Iterable `](/apis/Interfaces/HH/Iterable/).
+ ` int $len ` - The length of the returned [` Iterable `](/apis/Interfaces/HH/Iterable/).




## Returns




* [` Iterable<Tv> `](/apis/Interfaces/HH/Iterable/) - An [` Iterable `](/apis/Interfaces/HH/Iterable/) that is a proper subset of the current [` Iterable `](/apis/Interfaces/HH/Iterable/)
  starting at `` $start `` up to but not including the element
  ``` $start + $len ```.
<!-- HHAPIDOC -->
