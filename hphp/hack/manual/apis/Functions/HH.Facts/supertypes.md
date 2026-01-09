
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all types which the given type extends, implements, or uses




``` Hack
namespace HH\Facts;

function supertypes(
  classname<mixed> $derived_type,
  ?DeriveFilters<string, dynamic> $filters = NULL,
): vec<string, classname<mixed>>;
```




Throws InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<mixed> $derived_type `
+ ` ?DeriveFilters<string, dynamic> $filters = NULL `




## Returns




* ` vec<string, classname<mixed>> `
<!-- HHAPIDOC -->
