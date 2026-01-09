
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/docs/apis/Interfaces/ConstVector/) that is the concatenation of the values of the
current [` ConstMap `](/docs/apis/Interfaces/ConstMap/) and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): ConstVector<Tu>;
```




The provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is concatenated to the end of the current
[` ConstMap `](/docs/apis/Interfaces/ConstMap/) to produce the returned [` ConstVector `](/docs/apis/Interfaces/ConstVector/).




## Guide




+ [Constraints](</docs/hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/docs/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` ConstMap `](/docs/apis/Interfaces/ConstMap/).




## Returns




- [` ConstVector<Tu> `](/docs/apis/Interfaces/ConstVector/) - The integer-indexed concatenated [` ConstVector `](/docs/apis/Interfaces/ConstVector/).
<!-- HHAPIDOC -->
