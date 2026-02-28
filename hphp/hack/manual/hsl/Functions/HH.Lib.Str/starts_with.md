
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the string starts with the given prefix




``` Hack
namespace HH\Lib\Str;

function starts_with(
  string $string,
  string $prefix,
): bool;
```




For a case-insensitive check, see ` Str\starts_with_ci() `.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `
* ` string $prefix `




## Returns




- ` bool `




## Examples




``` basic-usage.hack
$result = Str\starts_with("example_string", "example");
echo($result);
//result: true

$result = Str\starts_with("example_string", "EXAMPLE");
echo($result);
//result: false

$result = Str\starts_with("example_string", "string");
echo($result);
//result: false
```
<!-- HHAPIDOC -->
