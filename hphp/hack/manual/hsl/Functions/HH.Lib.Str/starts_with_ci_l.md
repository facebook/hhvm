
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the string starts with the given prefix (case-insensitive)




``` Hack
namespace HH\Lib\Str;

function starts_with_ci_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  string $prefix,
): bool;
```




Locale-specific collation rules will be followed, and strings will be
normalized in encodings that support multiple representations of the same
characters, such as UTF8.




For a case-sensitive check, see ` Str\starts_with() `.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` string $prefix `




## Returns




- ` bool `
<!-- HHAPIDOC -->
