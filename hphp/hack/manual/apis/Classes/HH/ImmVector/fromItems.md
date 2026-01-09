
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) from the given [` Traversable `](/docs/apis/Interfaces/HH/Traversable/), or an empty
[` ImmVector `](/docs/apis/Classes/HH/ImmVector/) if `` null `` is passed




``` Hack
public static function fromItems(
  ?Traversable<Tv> $iterable,
): ImmVector<Tv>;
```




This is the static method version of the [` ImmVector::__construct() `](/docs/apis/Classes/HH/ImmVector/__construct/)
constructor.




## Parameters




+ ` ? `[` Traversable<Tv> `](/docs/apis/Interfaces/HH/Traversable/)`` $iterable ``




## Returns




* [` ImmVector<Tv> `](/docs/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) with the values from the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/); or an
  empty [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) if the [` Traversable `](/docs/apis/Interfaces/HH/Traversable/) is `` null ``.




## Examples




See [` Vector::fromItems `](/docs/apis/Classes/HH/Vector/fromItems/#examples) for usage examples.
<!-- HHAPIDOC -->
