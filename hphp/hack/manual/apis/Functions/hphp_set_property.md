
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_set_property() - Used by ReflectionProperty to set the value of a
property on an instance of a class




``` Hack
function hphp_set_property(
  object $obj,
  string $cls = '',
  string $prop,
  mixed $value,
): void;
```




## Parameters




+ ` object $obj ` - The object to set the property on.
+ ` string $cls = '' ` - The name of the class that the property is accessible
  in or null to only set a public property.
+ ` string $prop ` - The name of the property.
+ ` mixed $value ` - The value to set the property to.




## Returns




* ` void `
<!-- HHAPIDOC -->
