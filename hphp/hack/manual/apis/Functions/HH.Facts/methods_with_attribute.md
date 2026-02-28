
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all methods matching the given filters




``` Hack
namespace HH\Facts;

function methods_with_attribute(
  classname<\HH\MethodAttribute> $attribute,
): vec<string, (classname<nonnull>, string)>;
```




Throws InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<\HH\MethodAttribute> $attribute `




## Returns




* ` vec<string, (classname<nonnull>, string)> `
<!-- HHAPIDOC -->
