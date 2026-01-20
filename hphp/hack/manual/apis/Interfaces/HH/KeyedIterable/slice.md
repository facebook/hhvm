
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) starting from a given key
up to, but not including, the element at the provided length from the
starting key




``` Hack
public function slice(
  int $start,
  int $len,
): KeyedIterable<Tk, Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Interfaces/HH/KeyedIterable/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) will always be a proper subset of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).




## Parameters




+ ` int $start ` - The starting key of the current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) to begin
  the returned [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).
+ ` int $len ` - The length of the returned [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).




## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - A [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) that is a proper subset of the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) starting at `` $start `` up to but not including the
  element ``` $start + $len ```.
<!-- HHAPIDOC -->
