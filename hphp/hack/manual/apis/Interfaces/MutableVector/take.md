
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) containing the first `` n `` values of the current
[` MutableVector `](/docs/apis/Interfaces/MutableVector/)




``` Hack
public function take(
  int $n,
): MutableVector<Tv>;
```




The returned [` MutableVector `](/docs/apis/Interfaces/MutableVector/) will always be a proper subset of the current
[` MutableVector `](/docs/apis/Interfaces/MutableVector/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the returned
  [` MutableVector `](/docs/apis/Interfaces/MutableVector/).




## Returns




* [` MutableVector<Tv> `](/docs/apis/Interfaces/MutableVector/) - A [` MutableVector `](/docs/apis/Interfaces/MutableVector/) that is a proper subset of the current
  [` MutableVector `](/docs/apis/Interfaces/MutableVector/) up to `` n `` elements.
<!-- HHAPIDOC -->
