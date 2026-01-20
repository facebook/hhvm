
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first position of the "needle" string in the "haystack" string,
or null if it isn't found (case-insensitive)




``` Hack
namespace HH\Lib\Str;

function search_ci_l(
  \HH\Lib\Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
): ?int;
```




Locale-specific rules for case-insensitive comparisons will be used.




An optional offset determines where in the haystack the search begins. If the
offset is negative, the search will begin that many characters from the end
of the string. If the offset is out-of-bounds, a InvalidArgumentException will be
thrown.




+ To simply check if the haystack contains the needle, see ` Str\contains() `.
+ To get the case-sensitive position, see ` Str\search() `.
+ To get the last position of the needle, see ` Str\search_last() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $haystack `
- ` string $needle `
- ` int $offset = 0 `




## Returns




+ ` ?int `
<!-- HHAPIDOC -->
