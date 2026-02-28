
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an [` ImmSet `](/apis/Classes/HH/ImmSet/) from the keys of the specified container




``` Hack
public static function fromKeysOf<Tk as arraykey>(
  ?KeyedContainer<Tk, mixed> $container,
): ImmSet<Tk>;
```




The keys of the container will be the values of the [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Parameters




+ ` ? `[` KeyedContainer<Tk, `](/apis/Interfaces/HH/KeyedContainer/)`` mixed> $container `` - The container with the keys used to create the
  [` ImmSet `](/apis/Classes/HH/ImmSet/).




## Returns




* [` ImmSet<Tk> `](/apis/Classes/HH/ImmSet/) - An [` ImmSet `](/apis/Classes/HH/ImmSet/) built from the keys of the specified container.




## Examples




See [` Set::fromKeysOf `](/apis/Classes/HH/Set/fromKeysOf/#examples) for usage examples.
<!-- HHAPIDOC -->
