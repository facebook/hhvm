
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with the first character capitalized




``` Hack
namespace HH\Lib\Str;

function capitalize_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
): string;
```




If the first character is already capitalized or isn't alphabetic, the string
will be unchanged.




Locale-specific capitalization rules will be respected, e.g. ` i ` -> `` I `` vs
``` i ``` -> ```` İ ````.




+ To capitalize all characters, see ` Str\uppercase_l() `.
+ To capitalize all words, see ` Str\capitalize_words_l() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` \HH\Lib\Locale\Locale $locale `
- ` string $string `




## Returns




+ ` string `
<!-- HHAPIDOC -->
