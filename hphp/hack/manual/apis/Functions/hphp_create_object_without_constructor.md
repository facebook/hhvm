
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_create_object_without_constructor() - Used by ReflectionClass to create
a new instance of an object,
without calling the constructor




``` Hack
function hphp_create_object_without_constructor(
  string $name,
): object;
```




## Parameters




+ ` string $name ` - The name of the object to create.




## Returns




* ` object ` - - The newly created object
<!-- HHAPIDOC -->
