
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with the given suffix removed, or the string itself if
it doesn't end with the suffix




``` Hack
namespace HH\Lib\Str;

function strip_suffix(
  string $string,
  string $suffix,
): string;
```




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `
* ` string $suffix `




## Returns




- ` string `




## Examples




``` basic-usage.hack
$input = "\$100 USD";
$result = Str\strip_suffix($input, "USD");
echo "Strip suffix from $input : $result \n";

```
<!-- HHAPIDOC -->
