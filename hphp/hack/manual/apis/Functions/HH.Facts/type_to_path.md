
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return the only path defining a given symbol




``` Hack
namespace HH\Facts;

function type_to_path(
  string $type_name,
): ?string;
```




Return ` null ` if the symbol is not defined, or is defined in more than one
place.




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` string $type_name `




## Returns




* ` ?string `
<!-- HHAPIDOC -->
