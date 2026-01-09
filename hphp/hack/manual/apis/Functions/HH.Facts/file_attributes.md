
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all attributes on the given file




``` Hack
namespace HH\Facts;

function file_attributes(
  string $file,
): vec<string, classname<\HH\FileAttribute>>;
```




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` string $file `




## Returns




* ` vec<string, classname<\HH\FileAttribute>> `
<!-- HHAPIDOC -->
