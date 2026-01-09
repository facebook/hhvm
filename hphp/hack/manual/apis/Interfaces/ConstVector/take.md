
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) containing the first `` n `` values of the current
[` ConstVector `](/docs/apis/Interfaces/ConstVector/)




``` Hack
public function take(
  int $n,
): ConstVector<Tv>;
```




The returned [` ConstVector `](/docs/apis/Interfaces/ConstVector/) will always be a proper subset of the current
[` ConstVector `](/docs/apis/Interfaces/ConstVector/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the returned
  [` ConstVector `](/docs/apis/Interfaces/ConstVector/).




## Returns




* [` ConstVector<Tv> `](/docs/apis/Interfaces/ConstVector/) - A [` ConstVector `](/docs/apis/Interfaces/ConstVector/) that is a proper subset of the current
  [` ConstVector `](/docs/apis/Interfaces/ConstVector/) up to `` n `` elements.
<!-- HHAPIDOC -->
