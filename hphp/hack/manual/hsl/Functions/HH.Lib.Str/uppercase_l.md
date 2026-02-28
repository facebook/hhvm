
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with all alphabetic characters converted to uppercase




``` Hack
namespace HH\Lib\Str;

function uppercase_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
): string;
```




Locale-specific capitalization rules will be respected, e.g. ` i ` -> `` I `` vs
``` i ``` -> ```` İ ````.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `




## Returns




- ` string `




## Examples




``` basic-usage.hack
$locale = \HH\Lib\Locale\create("en_US.UTF-8");
$uppercase_string = Str\uppercase_l($locale, 'ifoo');
echo "Get uppercase string with en_US.UTF-8: $uppercase_string \n";
```
<!-- HHAPIDOC -->
