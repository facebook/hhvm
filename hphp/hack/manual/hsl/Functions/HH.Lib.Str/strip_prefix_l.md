
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with the given prefix removed, or the string itself if
it doesn't start with the prefix




``` Hack
namespace HH\Lib\Str;

function strip_prefix_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  string $prefix,
): string;
```




Strings will be normalized for comparison in encodings that support multiple
representations, such as UTF-8.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` string $prefix `




## Returns




- ` string `
<!-- HHAPIDOC -->
