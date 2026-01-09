
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the path which uniquely defines the given symbol




``` Hack
namespace HH;

function autoload_type_to_path(
  string $type,
): ?string;
```




Returns an absolute canonical path with all symlinks dereferenced.




Throws InvalidOperationException if native autoloading is disabled.




## Parameters




+ ` string $type `




## Returns




* ` ?string `
<!-- HHAPIDOC -->
