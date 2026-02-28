
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Throws an exception unless the current [` ImmSet `](/apis/Classes/HH/ImmSet/) or the [` Traversable `](/apis/Interfaces/HH/Traversable/) is
empty




``` Hack
public function zip<Tu>(
  Traversable<Tu> $traversable,
): ImmSet<HH\nothing>;
```




Since [` ImmSet `](/apis/Classes/HH/ImmSet/)s only support integers or strings as values, we cannot
have a [` Pair `](/apis/Classes/HH/Pair/) as an [` ImmSet `](/apis/Classes/HH/ImmSet/) value. So in order to avoid an
`` InvalidArgumentException ``, either the current [` ImmSet `](/apis/Classes/HH/ImmSet/) or the
[` Traversable `](/apis/Interfaces/HH/Traversable/) must be empty so that we actually return an empty [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Parameters




+ [` Traversable<Tu> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) to use to combine with the
  elements of the current [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Returns




* [` ImmSet<HH\nothing> `](/apis/Classes/HH/ImmSet/) - The [` ImmSet `](/apis/Classes/HH/ImmSet/) that combines the values of the current [` ImmSet `](/apis/Classes/HH/ImmSet/)
  with the provided [` Traversable `](/apis/Interfaces/HH/Traversable/); one of these must be empty or
  an exception is thrown.




## Examples




See [` Set::zip `](/apis/Classes/HH/Set/zip/#examples) for usage examples.
<!-- HHAPIDOC -->
