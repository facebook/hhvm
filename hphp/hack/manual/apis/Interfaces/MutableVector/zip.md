
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) where each element is a [` Pair `](/docs/apis/Classes/HH/Pair/) that combines the
element of the current [` MutableVector `](/docs/apis/Interfaces/MutableVector/) and the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): MutableVector<Pair<Tv, Tu>>;
```




If the number of elements of the [` MutableVector `](/docs/apis/Interfaces/MutableVector/) are not equal to the
number of elements in the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), then only the combined elements
up to and including the final element of the one with the least number of
elements is included.




## Parameters




+ [` Traversable<Tu> `](/docs/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of this [` MutableVector `](/docs/apis/Interfaces/MutableVector/).




## Returns




* [` MutableVector<Pair<Tv, `](/docs/apis/Interfaces/MutableVector/)`` Tu>> `` - The [` MutableVector `](/docs/apis/Interfaces/MutableVector/) that combines the values of the current
  [` MutableVector `](/docs/apis/Interfaces/MutableVector/) with the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/).
<!-- HHAPIDOC -->
