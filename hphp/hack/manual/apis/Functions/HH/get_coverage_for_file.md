
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Extract coverage data for the file at path $file




``` Hack
namespace HH;

function get_coverage_for_file(
  string $file,
): vec<int>;
```




The returned vector
contains a list of line numbers that were seen at least once while coverage
was enablgc_enabled for the file.




## Parameters




+ ` string $file `




## Returns




* ` a ` - list of covered line numbers
<!-- HHAPIDOC -->
