
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_create_object() - Used by ReflectionClass to create a new instance of an
object, including calling the constructor




``` Hack
function hphp_create_object<T>(
  string $class_name,
  array $params,
): T;
```




## Parameters




+ ` string $class_name `
+ ` array $params ` - The parameters to pass to the constructor.




## Returns




* ` object ` - - The newly created object
<!-- HHAPIDOC -->
