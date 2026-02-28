
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the length of the given string in characters




``` Hack
namespace HH\Lib\Str;

function length_l(
  \HH\Lib\Locale\Locale $locale,
  string $string,
): int;
```




This function may be ` O(1) ` or `` O(n) `` depending on the encoding specified
by the locale (LC_CTYPE).




See ` Str\length() ` (or pass `` Locale\c() ``) for the length in bytes.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` \HH\Lib\Locale\Locale $locale `
* ` string $string `




## Returns




- ` int `
<!-- HHAPIDOC -->
