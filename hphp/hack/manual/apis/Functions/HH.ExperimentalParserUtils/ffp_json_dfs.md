
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH\ExperimentalParserUtils;

function ffp_json_dfs(
  mixed $json,
  bool $right,
  (function(varray_or_darray): ?varray_or_darray) $predicate,
  (function(varray_or_darray): bool) $skip_node,
): ?varray_or_darray;
```




## Parameters




+ ` mixed $json `
+ ` bool $right `
+ ` (function(varray_or_darray): ?varray_or_darray) $predicate `
+ ` (function(varray_or_darray): bool) $skip_node `




## Returns




* ` ?varray_or_darray `
<!-- HHAPIDOC -->
