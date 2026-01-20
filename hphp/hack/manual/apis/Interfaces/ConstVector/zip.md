
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` ConstVector `](/apis/Interfaces/ConstVector/) where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
element of the current [` ConstVector `](/apis/Interfaces/ConstVector/) and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): ConstVector<Pair<Tv, Tu>>;
```




If the number of elements of the [` ConstVector `](/apis/Interfaces/ConstVector/) are not equal to the
number of elements in the [` Traversable `](/apis/Interfaces/HH/Traversable/), then only the combined elements
up to and including the final element of the one with the least number of
elements is included.




## Parameters




+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of this [` ConstVector `](/apis/Interfaces/ConstVector/).




## Returns




* [` ConstVector<Pair<Tv, `](/apis/Interfaces/ConstVector/)`` Tu>> `` - The [` ConstVector `](/apis/Interfaces/ConstVector/) that combines the values of the current
  [` ConstVector `](/apis/Interfaces/ConstVector/) with the provided [` Traversable `](/apis/Interfaces/HH/Traversable/).
<!-- HHAPIDOC -->
