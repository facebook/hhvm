
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the string starts with the given prefix




``` Hack
namespace HH\Lib\Str;

function starts_with_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  string $prefix,
): bool;
```




Strings will be normalized for comparison in encodings that support multiple
representations, such as UTF-8.




For a case-insensitive check, see ` Str\starts_with_ci_l() `.
For a byte-wise check, see `` Str\starts_with() ``




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` string $prefix `




## Returns




- ` bool `
<!-- HHAPIDOC -->
