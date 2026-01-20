
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For every element in the provided [` Traversable `](/apis/Interfaces/HH/Traversable/), stores a value into the
current collection associated with each key, overwriting the previous value
associated with the key




``` Hack
public function setAll(
  ?KeyedTraversable<Tk, Tv> $traversable,
): this;
```




If a key is not present the current Vector that is present in the
[` Traversable `](/apis/Interfaces/HH/Traversable/), an exception is thrown. If you want to add a value even if a
key is not present, use `` addAll() ``.




It the current collection, meaning changes made to the current collection
will be reflected in the returned collection.




## Parameters




+ ` ? `[` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv> $traversable `` - The [` Traversable `](/apis/Interfaces/HH/Traversable/) with the new values to set. If
  `` null `` is provided, no changes are made.




## Returns




* ` this ` - Returns itself.
<!-- HHAPIDOC -->
