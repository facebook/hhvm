
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an [` ImmSet `](/apis/Classes/HH/ImmSet/) from the given [` Traversable `](/apis/Interfaces/HH/Traversable/), or an empty [` ImmSet `](/apis/Classes/HH/ImmSet/) if
`` null `` is passed




``` Hack
public static function fromItems(
  ?Traversable<Tv> $iterable,
): ImmSet<Tv>;
```




This is the static method version of the [` ImmSet::__construct() `](/apis/Classes/HH/ImmSet/__construct/)
constructor.




## Parameters




+ ` ? `[` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` ImmSet<Tv> `](/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/apis/Classes/HH/ImmSet/) with the values from the [` Traversable `](/apis/Interfaces/HH/Traversable/); or an empty
  [` ImmSet `](/apis/Classes/HH/ImmSet/) if the [` Traversable `](/apis/Interfaces/HH/Traversable/) is `` null ``.




## Examples




See [` Set::fromItems `](/apis/Classes/HH/Set/fromItems/#examples) for usage examples.
<!-- HHAPIDOC -->
