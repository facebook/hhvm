
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` Iterable `](/apis/Interfaces/HH/Iterable/) where each element is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
element of the current [` Iterable `](/apis/Interfaces/HH/Iterable/) and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): Iterable<Pair<Tv, Tu>>;
```




If the number of elements of the [` Iterable `](/apis/Interfaces/HH/Iterable/) are not equal to the number of
elements in the [` Traversable `](/apis/Interfaces/HH/Traversable/), then only the combined elements up to and
including the final element of the one with the least number of elements
is included.




@param $traversable - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to use to combine with the
elements of the current [` Iterable `](/apis/Interfaces/HH/Iterable/).




@return - The [` Iterable `](/apis/Interfaces/HH/Iterable/) that combines the values of the current
`` Itearable `` with the provided [` Traversable `](/apis/Interfaces/HH/Traversable/).




## Parameters




+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``




## Returns




* [` Iterable<Pair<Tv, `](/apis/Interfaces/HH/Iterable/)`` Tu>> ``
<!-- HHAPIDOC -->
