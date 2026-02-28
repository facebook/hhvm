
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) that is the concatenation of the values of the
current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): Iterable<Tu>;
```




The values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) to produce the returned [` Iterable `](/apis/Interfaces/HH/Iterable/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/).




## Returns




- [` Iterable<Tu> `](/apis/Interfaces/HH/Iterable/) - The concatenated [` Iterable `](/apis/Interfaces/HH/Iterable/).
<!-- HHAPIDOC -->
