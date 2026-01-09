
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all attributes on the given type alias




``` Hack
namespace HH\Facts;

function type_alias_attributes(
  string $type_alias,
): vec<string, classname<\HH\TypeAliasAttribute>>;
```




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` string $type_alias `




## Returns




* ` vec<string, classname<\HH\TypeAliasAttribute>> `
<!-- HHAPIDOC -->
