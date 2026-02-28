
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For each path/hash pair in ` $pathsAndHashes `, parse the file on the
filesystem, or lookup the file with the given SHA1 hash, and return a dict
mapping each path to its contents




``` Hack
namespace HH\Facts;

function extract(
  vec<(?string)> $pathsAndHashes,
  ?string $root = NULL,
): dict<string, ?FileData>;
```




Each given path should be relative to the given ` $root `.




## Parameters




+ ` vec<(?string)> $pathsAndHashes `
+ ` ?string $root = NULL `




## Returns




* ` dict<string, ?FileData> `
<!-- HHAPIDOC -->
