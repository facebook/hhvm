
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableMap `](/apis/Interfaces/MutableMap/) containing the first `` n `` key/values of the current
[` MutableMap `](/apis/Interfaces/MutableMap/)




``` Hack
public function take(
  int $n,
): MutableMap<Tk, Tv>;
```




The returned [` MutableMap `](/apis/Interfaces/MutableMap/) will always be a proper subset of the current
[` MutableMap `](/apis/Interfaces/MutableMap/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the [` MutableMap `](/apis/Interfaces/MutableMap/).




## Returns




* [` MutableMap<Tk, `](/apis/Interfaces/MutableMap/)`` Tv> `` - A [` MutableMap `](/apis/Interfaces/MutableMap/) that is a proper subset of the current
  [` MutableMap `](/apis/Interfaces/MutableMap/) up to `` n `` elements.
<!-- HHAPIDOC -->
