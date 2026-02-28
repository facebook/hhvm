
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Add a value to the collection and return the collection itself




``` Hack
public function add(
  Te $e,
): this;
```




It returns the current collection, meaning changes made to the current
collection will be reflected in the returned collection.




## Parameters




+ ` Te $e ` - The value to add.




## Returns




* ` this ` - The updated collection itself.
<!-- HHAPIDOC -->
