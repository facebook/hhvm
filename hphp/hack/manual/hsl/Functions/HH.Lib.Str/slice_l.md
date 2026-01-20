
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a substring of length ` $length ` of the given string starting at the
`` $offset ``




``` Hack
namespace HH\Lib\Str;

function slice_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
  int $offset,
  ?int $length = NULL,
): string;
```




` $offset ` and `` $length `` are specified as a number of characters.




If no length is given, the slice will contain the rest of the
string. If the length is zero, the empty string will be returned. If the
offset is out-of-bounds, an InvalidArgumentException will be thrown.




See ` slice() ` for a byte-based operation.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `
* ` int $offset `
* ` ?int $length = NULL `




## Returns




- ` string `
<!-- HHAPIDOC -->
