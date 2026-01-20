
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) that is the concatenation of the values of the
current [` ConstSet `](/apis/Interfaces/ConstSet/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): ConstVector<Tu>;
```




The values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` ConstSet `](/apis/Interfaces/ConstSet/) to produce the returned [` ConstVector `](/apis/Interfaces/ConstVector/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` ConstSet `](/apis/Interfaces/ConstSet/).




## Returns




- [` ConstVector<Tu> `](/apis/Interfaces/ConstVector/) - The concatenated [` ConstVector `](/apis/Interfaces/ConstVector/).
<!-- HHAPIDOC -->
