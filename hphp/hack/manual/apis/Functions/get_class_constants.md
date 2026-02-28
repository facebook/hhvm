
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the constants of the given class




``` Hack
function get_class_constants(
  string $class_name,
): darray<string, mixed>;
```




## Parameters




+ ` string $class_name ` - The class name




## Returns




* ` array ` - - Returns an associative array of constants with their values.
<!-- HHAPIDOC -->
