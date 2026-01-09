
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) that is the concatenation of the values of the
current [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): ImmVector<Tu>;
```




The values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) to produce the returned [` ImmVector `](/docs/apis/Classes/HH/ImmVector/).




## Guide




+ [Constraints](</docs/hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/docs/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` ImmSet `](/docs/apis/Classes/HH/ImmSet/).




## Returns




- [` ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/) - The concatenated [` ImmVector `](/docs/apis/Classes/HH/ImmVector/).




## Examples




See [` Set::concat `](/docs/apis/Classes/HH/Set/concat/#examples) for usage examples.
<!-- HHAPIDOC -->
