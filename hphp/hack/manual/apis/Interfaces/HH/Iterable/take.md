
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) containing the first `` n `` values of the current
[` Iterable `](/apis/Interfaces/HH/Iterable/)




``` Hack
public function take(
  int $n,
): Iterable<Tv>;
```




The returned [` Iterable `](/apis/Interfaces/HH/Iterable/) will always be a proper subset of the current
[` Iterable `](/apis/Interfaces/HH/Iterable/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the returned
  [` Iterable `](/apis/Interfaces/HH/Iterable/).




## Returns




* [` Iterable<Tv> `](/apis/Interfaces/HH/Iterable/) - An [` Iterable `](/apis/Interfaces/HH/Iterable/)`` that is a proper subset of the current ``Iterable``` up to ```n` elements.
<!-- HHAPIDOC -->
