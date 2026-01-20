
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), append a value into the
current collection




``` Hack
public function addAll(
  ?Traversable<Te> $traversable,
): this;
```




It returns the current collection, meaning changes made to the current
collection will be reflected in the returned collection.




## Parameters




+ ` ? `[` Traversable<Te> `](/apis/Interfaces/HH/Traversable/)`` $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) with the new values to set. If
  `` null `` is provided, no changes are made.




## Returns




* ` this ` - Returns itself.
<!-- HHAPIDOC -->
