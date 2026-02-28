
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the first position of the "needle" string in the "haystack" string,
or null if it isn't found




``` Hack
namespace HH\Lib\Str;

function search_l(
  \HH\Lib\Locale\Locale $locale,
  string $haystack,
  string $needle,
  int $offset = 0,
): ?int;
```




An optional offset determines where in the haystack the search begins. If the
offset is negative, the search will begin that many characters from the end
of the string. If the offset is out-of-bounds, a InvalidArgumentException will be
thrown.




+ To simply check if the haystack contains the needle, see ` Str\contains_l() `.
+ To get the case-insensitive position, see ` Str\search_ci_l() `.
+ To get the last position of the needle, see ` Str\search_last_l() `.




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
