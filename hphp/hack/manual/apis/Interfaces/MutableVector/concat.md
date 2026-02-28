
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableVector `](/apis/Interfaces/MutableVector/) that is the concatenation of the values of the
current [` MutableVector `](/apis/Interfaces/MutableVector/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): MutableVector<Tu>;
```




The values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` MutableVector `](/apis/Interfaces/MutableVector/) to produce the returned [` MutableVector `](/apis/Interfaces/MutableVector/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` MutableVector `](/apis/Interfaces/MutableVector/).




## Returns




- [` MutableVector<Tu> `](/apis/Interfaces/MutableVector/) - The concatenated [` MutableVector `](/apis/Interfaces/MutableVector/).
<!-- HHAPIDOC -->
