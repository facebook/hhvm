
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return all the symbols defined in the given path




``` Hack
namespace HH\Facts;

function path_to_types(
  string $path,
): vec<string, classname<nonnull>>;
```




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` string $path `




## Returns




* ` vec<string, classname<nonnull>> `
<!-- HHAPIDOC -->
