
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all types which extend, implement, or use the given base type




``` Hack
namespace HH\Facts;

function subtypes<T>(
  classname<T> $base_type,
  ?DeriveFilters<string, dynamic> $filters = NULL,
): vec<string, classname<T>>;
```




Throws InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<T> $base_type `
+ ` ?DeriveFilters<string, dynamic> $filters = NULL `




## Returns




* ` vec<string, classname<T>> `
<!-- HHAPIDOC -->
