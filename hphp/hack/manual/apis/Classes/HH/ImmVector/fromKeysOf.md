
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) from the keys of the specified container




``` Hack
public static function fromKeysOf<Tk as arraykey>(
  ?KeyedContainer<Tk, mixed> $container,
): ImmVector<Tk>;
```




Every key in the provided [` KeyedContainer `](/docs/apis/Interfaces/HH/KeyedContainer/) will appear sequentially in the
returned [` ImmVector `](/docs/apis/Classes/HH/ImmVector/), with the next available integer key assigned to each.




## Parameters




+ ` ? `[` KeyedContainer<Tk, `](/docs/apis/Interfaces/HH/KeyedContainer/)`` mixed> $container `` - The container with the keys used to create the
  current [` ImmVector `](/docs/apis/Classes/HH/ImmVector/).




## Returns




* [` ImmVector<Tk> `](/docs/apis/Classes/HH/ImmVector/) - An [` ImmVector `](/docs/apis/Classes/HH/ImmVector/) built from the keys of the specified container.




## Examples




See [` Vector::fromKeysOf `](/docs/apis/Classes/HH/Vector/fromKeysOf/#examples) for usage examples.
<!-- HHAPIDOC -->
