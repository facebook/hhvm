
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) that is the concatenation of the values of the
current [` ImmVector `](/apis/Classes/HH/ImmVector/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): ImmVector<Tu>;
```




The returned [` ImmVector `](/apis/Classes/HH/ImmVector/) is created from the values of the current
[` ImmVector `](/apis/Classes/HH/ImmVector/), followed by the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/).




The returned [` ImmVector `](/apis/Classes/HH/ImmVector/) is a new object; the current [` ImmVector `](/apis/Classes/HH/ImmVector/) is
unchanged.




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Returns




- [` ImmVector<Tu> `](/apis/Classes/HH/ImmVector/) - A new [` ImmVector `](/apis/Classes/HH/ImmVector/) containing the values from `` $traversable ``
  concatenated to the values from the current [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Examples




See [` Vector::concat `](/apis/Classes/HH/Vector/concat/#examples) for usage examples.
<!-- HHAPIDOC -->
