
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the given string with whitespace stripped from the left




``` Hack
namespace HH\Lib\Str;

function trim_left(
  string $string,
  ?string $char_mask = NULL,
): string;
```




See ` Str\trim() ` for more details.




+ To strip from both ends, see ` Str\trim() `.
+ To only strip from the right, see ` Str\trim_right() `.
+ To strip a specific prefix (instead of all characters matching a mask),
  see ` Str\strip_prefix() `.




## Guide




* [String](</hack/built-in-types/string>)







## Parameters




- ` string $string `
- ` ?string $char_mask = NULL `




## Returns




+ ` string `
<!-- HHAPIDOC -->
