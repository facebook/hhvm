
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Collect coverage data for all files covered in this request as a map from
filepath to a list of covered lines




``` Hack
namespace HH;

function get_all_coverage_data(): dict<string, vec<int>>;
```




## Returns




+ ` a ` - map of filepath -> line vector
<!-- HHAPIDOC -->
