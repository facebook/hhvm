
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a [` MutableSet `](/apis/Interfaces/MutableSet/) where each value is a [` Pair `](/apis/Classes/HH/Pair/) that combines the
value of the current [` MutableSet `](/apis/Interfaces/MutableSet/) and the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): MutableSet<nothing>;
```




If the number of values of the current [` ConstMap `](/apis/Interfaces/ConstMap/) are not equal to the
number of elements in the [` Traversable `](/apis/Interfaces/HH/Traversable/), then only the combined elements
up to and including the final element of the one with the least number of
elements is included.




Note that some implementations of sets only support certain types of keys
(e.g., only ` int ` or `` string `` values allowed). In that case, this method
could thrown an exception since a [` Pair `](/apis/Classes/HH/Pair/) wouldn't be supported/




## Parameters




+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of the current [` MutableSet `](/apis/Interfaces/MutableSet/).




## Returns




* [` MutableSet<nothing> `](/apis/Interfaces/MutableSet/) - The [` MutableSet `](/apis/Interfaces/MutableSet/) that combines the values of the current
  [` MutableSet `](/apis/Interfaces/MutableSet/) with the provided [` Traversable `](/apis/Interfaces/HH/Traversable/).
<!-- HHAPIDOC -->
