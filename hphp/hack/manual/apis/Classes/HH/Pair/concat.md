
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an [` ImmVector `](/apis/Classes/HH/ImmVector/) that is the concatenation of the values of the
current [` Pair `](/apis/Classes/HH/Pair/) and the values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/)




``` Hack
public function concat<Tu super mixed>(
  Traversable<mixed, Tu> $traversable,
): ImmVector<mixed, Tu>;
```




The values of the provided [` Traversable `](/apis/Interfaces/HH/Traversable/) is concatenated to the end of the
current [` Pair `](/apis/Classes/HH/Pair/) to produce the returned [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Guide




+ [Constraints](</hack/generics/type-constraints>)







## Parameters




* [` Traversable<mixed, `](/apis/Interfaces/HH/Traversable/)`` Tu> $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to concatenate to the current
  [` Pair `](/apis/Classes/HH/Pair/).




## Returns




- [` ImmVector<mixed, `](/apis/Classes/HH/ImmVector/)`` Tu> `` - The concatenated [` ImmVector `](/apis/Classes/HH/ImmVector/).




## Examples




This example creates a new [` ImmVector `](/apis/Classes/HH/ImmVector/) by concatenating a [` Traversable `](/apis/Interfaces/HH/Traversable/) with the values in the [` Pair `](/apis/Classes/HH/Pair/).




``` basic-usage.hack
$p = Pair {'foo', -1.5};

$v = $p->concat(vec[100, 'bar']);
\var_dump($v);
```
<!-- HHAPIDOC -->
