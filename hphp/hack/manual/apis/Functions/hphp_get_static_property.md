
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_get_static_property() - Used by ReflectionProperty to get the value of a
static property on a class




``` Hack
function hphp_get_static_property(
  string $cls,
  string $prop,
  bool $force,
): mixed;
```




## Parameters




+ ` string $cls ` - The name of the class.
+ ` string $prop ` - The name of the static property
+ ` bool $force ` - Whether or not to get protected and private properties
  (true) or only public ones (false)




## Returns




* ` mixed ` - - The value of the property
<!-- HHAPIDOC -->
