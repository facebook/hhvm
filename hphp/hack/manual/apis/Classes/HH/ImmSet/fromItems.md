
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) from the given [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), or an empty [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) if
`` null `` is passed




``` Hack
public static function fromItems(
  ?Traversable<Tv> $iterable,
): ImmSet<Tv>;
```




This is the static method version of the [` ImmSet::__construct() `](/docs/apis/Classes/HH/ImmSet/__construct/)
constructor.




## Parameters




+ ` ? `[` Traversable<Tv> `](/docs/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` ImmSet<Tv> `](/docs/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) with the values from the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/); or an empty
  [` ImmSet `](/docs/apis/Classes/HH/ImmSet/) if the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is `` null ``.




## Examples




See [` Set::fromItems `](/docs/apis/Classes/HH/Set/fromItems/#examples) for usage examples.
<!-- HHAPIDOC -->
