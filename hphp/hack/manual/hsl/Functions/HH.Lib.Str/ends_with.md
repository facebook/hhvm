
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the string ends with the given suffix




``` Hack
namespace HH\Lib\Str;

function ends_with(
  string $string,
  string $suffix,
): bool;
```




For a case-insensitive check, see ` Str\ends_with_ci() `.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `
* ` string $suffix `




## Returns




- ` bool `




## Examples




``` basic-usage.hack
$result = Str\ends_with("example_string", "string");
echo($result);
//result: true

$result = Str\ends_with("example_string", "STRING");
echo($result);
//result: false

$result = Str\ends_with("example_string", "uncontained");
echo($result);
// result false
```
<!-- HHAPIDOC -->
