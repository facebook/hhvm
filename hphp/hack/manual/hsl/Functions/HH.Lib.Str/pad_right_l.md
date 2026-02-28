
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the string padded to the total length (in characters) by appending
the ` $pad_string ` to the right




``` Hack
namespace HH\Lib\Str;

function pad_right_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  int $total_length,
  string $pad_string = ' ',
): string;
```




If the length of the input string plus the pad string exceeds the total
length, the pad string will be truncated. If the total length is less than or
equal to the length of the input string, no padding will occur.




To pad the string on the left, see ` Str\pad_left() `.
To pad the string to a fixed number of bytes, see `` Str\pad_right() ``




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` int $total_length `
* ` string $pad_string = ' ' `




## Returns




- ` string `
<!-- HHAPIDOC -->
