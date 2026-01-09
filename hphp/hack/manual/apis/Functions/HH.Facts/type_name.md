
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Resolve a string into a classname that's properly capitalized and properly
typed




``` Hack
namespace HH\Facts;

function type_name(
  string $classname,
): ?classname<nonnull>;
```




Return ` null ` if the classname does not exist in the codebase, even with
different capitalization.




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` string $classname `




## Returns




* ` ?classname<nonnull> `
<!-- HHAPIDOC -->
