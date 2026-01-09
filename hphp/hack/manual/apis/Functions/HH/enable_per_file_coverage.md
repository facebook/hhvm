
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Begin collecting code coverage on all subsequent calls into files in $files
during this request




``` Hack
namespace HH;

function enable_per_file_coverage(
  keyset<string> $files,
): void;
```




The requst must be executing in non-RepoAuthoritative mode and the server
must be configured with Eval.EnablePerFileCoverage = true.




## Parameters




+ ` keyset<string> $files ` a list of paths to collect coverage from




## Returns




* ` void `
<!-- HHAPIDOC -->
