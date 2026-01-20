
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with all alphabetic characters converted to lowercase




``` Hack
namespace HH\Lib\Str;

function lowercase_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
): string;
```




Locale-specific capitalization rules will be respected, e.g. ` I ` -> `` i `` vs
``` I ``` -> ```` ı ````




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `




## Returns




- ` string `
<!-- HHAPIDOC -->
