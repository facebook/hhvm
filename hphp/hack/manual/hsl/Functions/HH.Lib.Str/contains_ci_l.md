
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether the "haystack" string contains the "needle" string
(case-insensitive)




``` Hack
namespace HH\Lib\Str;

function contains_ci_l(
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




Locale-specific rules for case-insensitive comparisons will be used, and
strings will be normalized before comparing if the locale specifies an
encoding that supports multiple representations of the same characters, such
as UTF-8.




+ To search for the needle case-sensitively, see ` Str\contains_l() `.
+ To get the position of the needle case-insensitively, see ` Str\search_ci_l() `.




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
