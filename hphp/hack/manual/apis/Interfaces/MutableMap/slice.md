
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` MutableMap `](/apis/Interfaces/MutableMap/) starting from a given key
location up to, but not including, the element at the provided length from
the starting key location




``` Hack
public function slice(
  int $start,
  int $len,
): MutableMap<Tk, Tv>;
```




` $start ` is 0-based. `` $len `` is 1-based. So [` slice(0, `](/apis/Interfaces/MutableMap/slice/)`` 2) `` would return the
keys and values at key location 0 and 1.




The returned [` MutableMap `](/apis/Interfaces/MutableMap/) will always be a proper subset of the current
[` MutableMap `](/apis/Interfaces/MutableMap/).




## Parameters




+ ` int $start ` - The starting key location of the current [` MutableMap `](/apis/Interfaces/MutableMap/) for
  the feturned [` MutableMap `](/apis/Interfaces/MutableMap/).
+ ` int $len ` - The length of the returned [` MutableMap `](/apis/Interfaces/MutableMap/).




## Returns




* [` MutableMap<Tk, `](/apis/Interfaces/MutableMap/)`` Tv> `` - A [` MutableMap `](/apis/Interfaces/MutableMap/) that is a proper subset of the current
  [` MutableMap `](/apis/Interfaces/MutableMap/) starting at `` $start `` up to but not including the
  element ``` $start + $len ```.
<!-- HHAPIDOC -->
