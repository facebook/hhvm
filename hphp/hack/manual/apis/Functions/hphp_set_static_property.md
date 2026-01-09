
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_set_static_property() - Used by ReflectionProperty to set the value of a
static property on a class




``` Hack
function hphp_set_static_property(
  string $cls,
  string $prop,
  mixed $value,
  bool $force,
): void;
```




## Parameters




+ ` string $cls ` - The name of the class.
+ ` string $prop ` - The name of the static property
+ ` mixed $value ` - The value to set the property to
+ ` bool $force ` - Whether or not to set protected and private properties
  (true) or only public ones (false)




## Returns




* ` void `
<!-- HHAPIDOC -->
