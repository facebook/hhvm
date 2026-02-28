
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with all alphabetic characters converted to uppercase




``` Hack
namespace HH\Lib\Str;

function uppercase(
  string $string,
): string;
```




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` string $string `




## Returns




- ` string `




## Examples




``` basic-usage.hack
$string = "hhvm";

$string_upper = Str\uppercase($string);
echo "Uppercase string: $string_upper \n";
//result: Uppercase string: HHVM
```
<!-- HHAPIDOC -->
