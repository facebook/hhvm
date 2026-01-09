
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/docs/apis/Interfaces/MutableVector/) that is the concatenation of the values of the
current [` MutableSet `](/docs/apis/Interfaces/MutableSet/) and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): MutableVector<Tu>;
```




The values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` MutableSet `](/docs/apis/Interfaces/MutableSet/) to produce the returned [` MutableVector `](/docs/apis/Interfaces/MutableVector/).




## Guide




+ [Constraints](</docs/hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/docs/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` MutableSet `](/docs/apis/Interfaces/MutableSet/).




## Returns




- [` MutableVector<Tu> `](/docs/apis/Interfaces/MutableVector/) - The concatenated [` MutableVector `](/docs/apis/Interfaces/MutableVector/).
<!-- HHAPIDOC -->
