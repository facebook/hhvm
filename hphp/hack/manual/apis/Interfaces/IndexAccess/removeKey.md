
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Removes the specified key (and associated value) from the current
collection




``` Hack
public function removeKey(
  Tk $k,
): this;
```




If the key is not in the current collection, the current collection is
unchanged.




It the current collection, meaning changes made to the current collection
will be reflected in the returned collection.




## Parameters




+ ` Tk $k ` - The key to remove.




## Returns




* ` this ` - Returns itself.
<!-- HHAPIDOC -->
