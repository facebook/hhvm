
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) that is the concatenation of the values of the
current [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) and the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super Tv>(
  Traversable<Tu> $traversable,
): ImmVector<Tu>;
```




The returned [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) is created from the values of the current
[` ImmVector `](/docs/apis/Classes/HH/ImmVector/), followed by the values of the provided [` Traversable `](/docs/apis/Interfaces/HH/Traversable/).




The returned [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) is a new object; the current [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) is
unchanged.




## Guide




+ [Constraints](</docs/hack/generics/type-constraints>)







## Parameters




* [` Traversable<Tu> `](/docs/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` ImmVector `](/docs/apis/Classes/HH/ImmVector/).




## Returns




- [` ImmVector<Tu> `](/docs/apis/Classes/HH/ImmVector/) - A new [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) containing the values from `` $traversable ``
  concatenated to the values from the current [` ImmVector `](/docs/apis/Classes/HH/ImmVector/).




## Examples




See [` Vector::concat `](/docs/apis/Classes/HH/Vector/concat/#examples) for usage examples.
<!-- HHAPIDOC -->
