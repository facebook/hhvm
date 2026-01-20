
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the "haystack" string contains the "needle" string




``` Hack
namespace HH\Lib\Str;

function contains_l(
  \HH\Lib\Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
): bool;
```




An optional offset determines where in the haystack the search begins. If the
offset is negative, the search will begin that many characters from the end
of the string. If the offset is out-of-bounds, a InvalidArgumentException will be
thrown.




Strings will be normalized for comparison in encodings that support multiple
representations, such as UTF-8.




+ To get the position of the needle, see ` Str\search_l() `.
+ To search for the needle case-insensitively, see ` Str\contains_ci_l() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $haystack `
- ` string $needle `
- ` int $offset = 0 `




## Returns




+ ` bool `
<!-- HHAPIDOC -->
