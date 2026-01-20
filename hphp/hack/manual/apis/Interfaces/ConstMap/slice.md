
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` ConstMap `](/apis/Interfaces/ConstMap/) starting from a given key
location up to, but not including, the element at the provided length from
the starting key location




``` Hack
public function slice(
  int $start,
  int $len,
): ConstMap<Tk, Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Interfaces/ConstMap/slice/)`` 2) `` would return the
keys and values at key location 0 and 1.




The returned [` ConstMap `](/apis/Interfaces/ConstMap/) will always be a proper subset of the current
[` ConstMap `](/apis/Interfaces/ConstMap/).




## Parameters




+ ` int $start ` - The starting key location of the current [` ConstMap `](/apis/Interfaces/ConstMap/) for the
  returned [` ConstMap `](/apis/Interfaces/ConstMap/).
+ ` int $len ` - The length of the returned [` ConstMap `](/apis/Interfaces/ConstMap/).




## Returns




* [` ConstMap<Tk, `](/apis/Interfaces/ConstMap/)`` Tv> `` - A [` ConstMap `](/apis/Interfaces/ConstMap/) that is a proper subset of the current [` ConstMap `](/apis/Interfaces/ConstMap/)
  starting at `` $start `` up to but not including the element
  ``` $start + $len ```.
<!-- HHAPIDOC -->
