
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/docs/apis/Interfaces/MutableSet/) containing the first `` n `` values of the current
[` MutableSet `](/docs/apis/Interfaces/MutableSet/)




``` Hack
public function take(
  int $n,
): MutableSet<Tv>;
```




The returned [` MutableSet `](/docs/apis/Interfaces/MutableSet/) will always be a proper subset of the current
[` MutableSet `](/docs/apis/Interfaces/MutableSet/).




` $n ` is 1-based. So the first element is 1, the second 2, etc.




## Parameters




+ ` int $n ` - The last element that will be included in the [` MutableSet `](/docs/apis/Interfaces/MutableSet/).




## Returns




* [` MutableSet<Tv> `](/docs/apis/Interfaces/MutableSet/) - A [` MutableSet `](/docs/apis/Interfaces/MutableSet/) that is a proper subset of the current
  [` MutableSet `](/docs/apis/Interfaces/MutableSet/) up to `` n `` elements.
<!-- HHAPIDOC -->
