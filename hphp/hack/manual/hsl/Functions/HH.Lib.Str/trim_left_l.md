
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the given string with whitespace stripped from the left




``` Hack
namespace HH\Lib\Str;

function trim_left_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  ?string $char_mask = NULL,
): string;
```




See ` Str\trim_l() ` for more details.




+ To strip from both ends, see ` Str\trim_l() `.
+ To only strip from the right, see ` Str\trim_right_l() `.
+ To strip a specific prefix (instead of all characters matching a mask),
  see ` Str\strip_prefix_l() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $string `
- ` ?string $char_mask = NULL `




## Returns




+ ` string `
<!-- HHAPIDOC -->
