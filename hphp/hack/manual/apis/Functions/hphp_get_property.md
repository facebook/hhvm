
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_get_property() - Used by ReflectionProperty to get the value of a
property on an instance of a class




``` Hack
function hphp_get_property(
  object $obj,
  string $cls = '',
  string $prop,
): mixed;
```




## Parameters




+ ` object $obj ` - The object to get the property from.
+ ` string $cls = '' ` - The name of the class that the property is accessible
  in or null to only get a public property.
+ ` string $prop ` - The name of the property.




## Returns




* ` mixed ` - - The value of the property.
<!-- HHAPIDOC -->
