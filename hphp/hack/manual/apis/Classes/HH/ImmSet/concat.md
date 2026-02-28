
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) that is the concatenation of the values of the
current [` ImmSet `](/apis/Classes/HH/ImmSet/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): ImmVector<Tu>;
```




The values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` ImmSet `](/apis/Classes/HH/ImmSet/) to produce the returned [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Returns




- [` ImmVector<Tu> `](/apis/Classes/HH/ImmVector/) - The concatenated [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Examples




See [` Set::concat `](/apis/Classes/HH/Set/concat/#examples) for usage examples.
<!-- HHAPIDOC -->
