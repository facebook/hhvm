
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string with all words capitalized




``` Hack
namespace HH\Lib\Str;

function capitalize_words(
  string $string,
  ?string $delimiters = NULL,
): string;
```




Words are delimited by space, tab, newline, carriage return, form-feed, and
vertical tab by default, but you can specify custom delimiters.




+ To capitalize all characters, see ` Str\uppercase() `.
+ To capitalize only the first character, see ` Str\capitalize() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $string `
- ` ?string $delimiters = NULL `




## Returns




+ ` string `
<!-- HHAPIDOC -->
