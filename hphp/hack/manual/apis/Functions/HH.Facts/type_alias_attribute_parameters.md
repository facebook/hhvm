
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all parameters for the given attribute on the given type alias




``` Hack
namespace HH\Facts;

function type_alias_attribute_parameters(
  string $type_alias,
  classname<\HH\TypeAliasAttribute> $attribute,
): vec<dynamic>;
```




Return an empty vec if the type alias doesn't exist or doesn't have the
given attribute.




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` string $type_alias `
+ ` classname<\HH\TypeAliasAttribute> $attribute `




## Returns




* ` vec<dynamic> `
<!-- HHAPIDOC -->
