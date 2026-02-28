
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the given string with whitespace stripped from the right




``` Hack
namespace HH\Lib\Str;

function trim_right_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  ?string $char_mask = NULL,
): string;
```




See ` Str\trim_l ` for more details.




+ To strip from both ends, see ` Str\trim_l() `.
+ To only strip from the left, see ` Str\trim_left_l() `.
+ To strip a specific suffix (instead of all characters matching a mask),
  see ` Str\strip_suffix_l() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $string `
- ` ?string $char_mask = NULL `




## Returns




+ ` string `
<!-- HHAPIDOC -->
