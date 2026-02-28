
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) containing the first `` n `` values of the current
[` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)




``` Hack
public function take(
  int $n,
): KeyedIterable<Tk, Tv>;
```




The returned [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) will always be a proper subset of the current
[` Iterable `](/apis/Interfaces/HH/Iterable/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the returned
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).




## Returns




* [` KeyedIterable<Tk, `](/apis/Interfaces/HH/KeyedIterable/)`` Tv> `` - A [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/)`` that is a proper subset of the current ``KeyedIterable``` up to ```n` elements.
<!-- HHAPIDOC -->
