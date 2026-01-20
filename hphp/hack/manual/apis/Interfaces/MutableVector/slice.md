
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a subset of the current [` MutableVector `](/apis/Interfaces/MutableVector/) starting from a given key
up to, but not including, the element at the provided length from the
starting key




``` Hack
public function slice(
  int $start,
  int $len,
): MutableVector<Tv>;
```




` $start ` is 0-based. $len is 1-based. So [` slice(0, `](/apis/Interfaces/MutableVector/slice/)`` 2) `` would return the
elements at key 0 and 1.




The returned [` MutableVector `](/apis/Interfaces/MutableVector/) will always be a proper subset of this
[` MutableVector `](/apis/Interfaces/MutableVector/).




## Parameters




+ ` int $start ` - The starting key of this Vector to begin the returned
  [` MutableVector `](/apis/Interfaces/MutableVector/).
+ ` int $len ` - The length of the returned [` MutableVector `](/apis/Interfaces/MutableVector/).




## Returns




* [` MutableVector<Tv> `](/apis/Interfaces/MutableVector/) - A [` MutableVector `](/apis/Interfaces/MutableVector/) that is a proper subset of the current
  [` MutableVector `](/apis/Interfaces/MutableVector/) starting at `` $start `` up to but not including the
  element ``` $start + $len ```.
<!-- HHAPIDOC -->
