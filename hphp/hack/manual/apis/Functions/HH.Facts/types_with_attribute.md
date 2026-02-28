
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all types matching the given filters




``` Hack
namespace HH\Facts;

function types_with_attribute(
  classname<\HH\ClassLikeAttribute> $attribute,
): vec<string, classname<mixed>>;
```




Throws InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<\HH\ClassLikeAttribute> $attribute `




## Returns




* ` vec<string, classname<mixed>> `
<!-- HHAPIDOC -->
