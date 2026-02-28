
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns ` < 0 ` if `` $string1 `` is less than ``` $string2 ```, ```` > 0 ```` if ````` $string1 ````` is
greater than `````` $string2 ``````, and ``````` 0 ``````` if they are equal




``` Hack
namespace HH\Lib\Str;

function compare_l(
  \HH\Lib\Locale\Locale $locale,
  string $string1,
  string $string2,
): int;
```




For a case-insensitive comparison, see ` Str\compare_ci_l() `.




Locale-specific collation rules will be followed, and strings will be
normalized in encodings that support multiple representations of the same
characters, such as UTF8.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string1 `
* ` string $string2 `




## Returns




- ` int `
<!-- HHAPIDOC -->
