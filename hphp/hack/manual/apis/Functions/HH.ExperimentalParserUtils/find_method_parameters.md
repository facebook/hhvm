
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Instead of doing a full recursion like the lambda extractor, this function
can do a shallow search of the tree to collect methods by name




``` Hack
namespace HH\ExperimentalParserUtils;

function find_method_parameters(
  \HH\ParseTree $json,
  string $method_name,
  int $line_number,
): MethodParametersNode;
```




If there is a tie, use the line number




## Parameters




+ ` \HH\ParseTree $json `
+ ` string $method_name `
+ ` int $line_number `




## Returns




* ` MethodParametersNode `
<!-- HHAPIDOC -->
